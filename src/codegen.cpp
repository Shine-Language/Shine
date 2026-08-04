#include "shine/codegen.h"
#include <llvm/IR/Verifier.h>
#include "shine/error.h"

namespace shine {

CodeGen::CodeGen()
    : ctx_(std::make_unique<llvm::LLVMContext>()),
      mod_(std::make_unique<llvm::Module>("shine_module", *ctx_)),
      b_(std::make_unique<llvm::IRBuilder<>>(*ctx_)) {}

llvm::Type* CodeGen::mapType(const TypeRef& t) {
    if (t.name == "int") return llvm::Type::getInt32Ty(*ctx_);
    if (t.name == "void") return llvm::Type::getVoidTy(*ctx_);
    throw CompileError(t.loc, "unknown type '" + t.name + "'");
}

llvm::AllocaInst* CodeGen::createAlloca(llvm::Function* f, llvm::Type* ty, const std::string& name) {
    llvm::IRBuilder<> tmp(&f->getEntryBlock(), f->getEntryBlock().begin());
    return tmp.CreateAlloca(ty, nullptr, name);
}

llvm::Function* CodeGen::putsFn() {
    if (puts_) return puts_;
    auto* ty = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx_),
                                        {llvm::PointerType::getUnqual(*ctx_)}, false);
    puts_ = llvm::Function::Create(ty, llvm::Function::ExternalLinkage, "puts", mod_.get());
    return puts_;
}

llvm::Function* CodeGen::getcharFn() {
    if (getchar_) return getchar_;
    auto* ty = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx_), false);
    getchar_ = llvm::Function::Create(ty, llvm::Function::ExternalLinkage, "getchar", mod_.get());
    return getchar_;
}

void CodeGen::declareFn(const FunctionDecl& fn) {
    std::vector<llvm::Type*> paramTys;
    for (auto& p : fn.params) paramTys.push_back(mapType(p.type));
    auto* ty = llvm::FunctionType::get(mapType(fn.returnType), paramTys, false);
    fns_[fn.name] = llvm::Function::Create(ty, llvm::Function::ExternalLinkage, fn.name, mod_.get());
}

void CodeGen::defineFn(const FunctionDecl& fn) {
    llvm::Function* f = fns_.at(fn.name);
    b_->SetInsertPoint(llvm::BasicBlock::Create(*ctx_, "entry", f));

    vars_.clear();
    size_t i = 0;
    for (auto& arg : f->args()) {
        arg.setName(fn.params[i].name);
        auto* slot = createAlloca(f, arg.getType(), fn.params[i].name);
        b_->CreateStore(&arg, slot);
        vars_[fn.params[i].name] = {slot, false};
        i++;
    }

    for (auto& s : fn.body) genStmt(*s);

    if (!b_->GetInsertBlock()->getTerminator()) {
        if (f->getReturnType()->isVoidTy()) b_->CreateRetVoid();
        else b_->CreateRet(llvm::Constant::getNullValue(f->getReturnType()));
    }

    std::string errStr;
    llvm::raw_string_ostream os(errStr);
    if (llvm::verifyFunction(*f, &os))
        throw CompileError(fn.loc, "codegen error in '" + fn.name + "': " + os.str());
}

void CodeGen::genStmt(const Stmt& s) {
    if (auto* r = dynamic_cast<const ReturnStmt*>(&s)) {
        if (r->value) b_->CreateRet(genExpr(*r->value));
        else b_->CreateRetVoid();
        return;
    }
    if (auto* v = dynamic_cast<const VarDeclStmt*>(&s)) {
        if (vars_.find(v->name) != vars_.end())
            throw CompileError(v->loc, "variable '" + v->name + "' is already declared");
        if (v->type.name == "void") throw CompileError(v->type.loc, "variables cannot have type void");
        auto* slot = createAlloca(b_->GetInsertBlock()->getParent(), mapType(v->type), v->name);
        b_->CreateStore(genExpr(*v->value), slot);
        vars_[v->name] = {slot, v->isMutable};
        return;
    }
    if (auto* a = dynamic_cast<const AssignStmt*>(&s)) {
        auto it = vars_.find(a->name);
        if (it == vars_.end()) throw CompileError(a->loc, "undeclared identifier '" + a->name + "'");
        if (!it->second.isMutable) throw CompileError(a->loc, "cannot assign to immutable variable '" + a->name + "'");
        b_->CreateStore(genExpr(*a->value), it->second.value);
        return;
    }
    if (auto* e = dynamic_cast<const ExprStmt*>(&s)) { genExpr(*e->expr); return; }
    throw CompileError(s.loc, "unhandled statement");
}

llvm::Value* CodeGen::genExpr(const Expr& e) {
    if (auto* i = dynamic_cast<const IntLiteralExpr*>(&e))
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx_), i->value, true);
    if (auto* s = dynamic_cast<const StringLiteralExpr*>(&e))
        return b_->CreateGlobalString(s->value, "str");
    if (auto* id = dynamic_cast<const IdentifierExpr*>(&e)) return genIdentifier(*id);
    if (auto* bin = dynamic_cast<const BinaryExpr*>(&e)) return genBinary(*bin);
    if (auto* c = dynamic_cast<const CallExpr*>(&e)) return genCall(*c);
    throw CompileError(e.loc, "unhandled expression");
}

llvm::Value* CodeGen::genIdentifier(const IdentifierExpr& i) {
    auto it = vars_.find(i.name);
    if (it == vars_.end()) throw CompileError(i.loc, "undeclared identifier '" + i.name + "'");
    return b_->CreateLoad(it->second.value->getAllocatedType(), it->second.value, i.name);
}

llvm::Value* CodeGen::genBinary(const BinaryExpr& e) {
    auto* left = genExpr(*e.left);
    auto* right = genExpr(*e.right);

    if (e.op == "+") return b_->CreateAdd(left, right, "addtmp");
    if (e.op == "-") return b_->CreateSub(left, right, "subtmp");
    if (e.op == "*") return b_->CreateMul(left, right, "multmp");
    if (e.op == "/") return b_->CreateSDiv(left, right, "divtmp");

    llvm::Value* cmp = nullptr;
    if (e.op == "==") cmp = b_->CreateICmpEQ(left, right, "eqtmp");
    else if (e.op == "!=") cmp = b_->CreateICmpNE(left, right, "netmp");
    else if (e.op == "<") cmp = b_->CreateICmpSLT(left, right, "lttmp");
    else if (e.op == "<=") cmp = b_->CreateICmpSLE(left, right, "letmp");
    else if (e.op == ">") cmp = b_->CreateICmpSGT(left, right, "gttmp");
    else if (e.op == ">=") cmp = b_->CreateICmpSGE(left, right, "getmp");

    if (cmp) return b_->CreateIntCast(cmp, llvm::Type::getInt32Ty(*ctx_), false, "booltmp");
    throw CompileError(e.loc, "unknown binary operator '" + e.op + "'");
}

llvm::Value* CodeGen::genWrite(const CallExpr& c) {
    if (c.args.size() != 1) throw CompileError(c.loc, "write() takes exactly 1 argument");
    auto* s = dynamic_cast<const StringLiteralExpr*>(c.args[0].get());
    if (!s) throw CompileError(c.args[0]->loc, "write() only supports string literals for now");
    return b_->CreateCall(putsFn(), {b_->CreateGlobalString(s->value, "str")});
}

llvm::Value* CodeGen::genTerminalPause(const CallExpr& c) {
    if (c.args.size() > 1) throw CompileError(c.loc, "terminal.pause() takes at most 1 argument");
    if (c.args.size() == 1 && !dynamic_cast<const IdentifierExpr*>(c.args[0].get()))
        throw CompileError(c.args[0]->loc, "terminal.pause() argument is just a placeholder name");
    b_->CreateCall(putsFn(), {b_->CreateGlobalString("Press Enter to continue...", "str")});
    return b_->CreateCall(getcharFn(), {});
}

llvm::Value* CodeGen::genCall(const CallExpr& c) {
    if (c.callee == "write") return genWrite(c);
    if (c.callee == "terminal.pause") return genTerminalPause(c);
    auto it = fns_.find(c.callee);
    if (it == fns_.end()) throw CompileError(c.loc, "call to undeclared function '" + c.callee + "'");

    llvm::Function* f = it->second;
    if (c.args.size() != f->arg_size())
        throw CompileError(c.loc, "'" + c.callee + "' expects " + std::to_string(f->arg_size()) +
                                       " argument(s), got " + std::to_string(c.args.size()));

    std::vector<llvm::Value*> args;
    for (auto& a : c.args) args.push_back(genExpr(*a));
    return b_->CreateCall(f, args);
}

std::unique_ptr<llvm::Module> CodeGen::generate(const Module& mod) {
    mod_->setSourceFileName(mod.file);
    for (auto& fn : mod.functions) declareFn(fn);
    for (auto& fn : mod.functions) defineFn(fn);
    return std::move(mod_);
}

}

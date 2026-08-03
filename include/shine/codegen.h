#pragma once
#include <memory>
#include <unordered_map>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "shine/ast.h"

namespace shine {

class CodeGen {
public:
    CodeGen();
    std::unique_ptr<llvm::Module> generate(const Module& mod);

private:
    void declareFn(const FunctionDecl& fn);
    void defineFn(const FunctionDecl& fn);
    void genStmt(const Stmt& s);
    llvm::Value* genExpr(const Expr& e);
    llvm::Value* genIdentifier(const IdentifierExpr& i);
    llvm::Value* genCall(const CallExpr& c);
    llvm::Value* genWrite(const CallExpr& c);
    llvm::Value* genTerminalPause(const CallExpr& c);
    llvm::Type* mapType(const TypeRef& t);
    llvm::Function* putsFn();
    llvm::Function* getcharFn();

    std::unique_ptr<llvm::LLVMContext> ctx_;
    std::unique_ptr<llvm::Module> mod_;
    std::unique_ptr<llvm::IRBuilder<>> b_;
    std::unordered_map<std::string, llvm::Function*> fns_;
    std::unordered_map<std::string, llvm::Value*> vars_;
    llvm::Function* puts_ = nullptr;
    llvm::Function* getchar_ = nullptr;
};

}

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include "shine/codegen.h"
#include "shine/error.h"
#include "shine/lexer.h"
#include "shine/parser.h"

using namespace shine;

static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) { std::cerr << "shinec: can't open '" << path << "'\n"; std::exit(1); }
    std::ostringstream ss; ss << f.rdbuf();
    return ss.str();
}

static std::string stripExt(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return path;
    return path.substr(0, dot);
}

template <class Mod>
static auto setTriple(Mod& mod, const llvm::Triple& t, int)
    -> decltype(mod.setTargetTriple(t), void()) {
    mod.setTargetTriple(t);
}

template <class Mod>
static void setTriple(Mod& mod, const llvm::Triple& t, long) {
    mod.setTargetTriple(t.getTriple());
}

template <class Tgt>
static auto makeTargetMachine(const Tgt& target, const llvm::Triple& t, const llvm::TargetOptions& opts, int)
    -> decltype(target.createTargetMachine(t, "generic", "", opts, llvm::Reloc::PIC_)) {
    return target.createTargetMachine(t, "generic", "", opts, llvm::Reloc::PIC_);
}

template <class Tgt>
static llvm::TargetMachine* makeTargetMachine(const Tgt& target, const llvm::Triple& t,
                                               const llvm::TargetOptions& opts, long) {
    return target.createTargetMachine(t.getTriple(), "generic", "", opts, llvm::Reloc::PIC_);
}

static void emitObj(llvm::Module& mod, const std::string& objPath) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    setTriple(mod, triple, 0);

    std::string lookupErr;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple.getTriple(), lookupErr);
    if (!target) { std::cerr << "shinec: " << lookupErr << "\n"; std::exit(1); }

    llvm::TargetOptions opts;
    std::unique_ptr<llvm::TargetMachine> tm(makeTargetMachine(*target, triple, opts, 0));
    mod.setDataLayout(tm->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(objPath, ec, llvm::sys::fs::OF_None);
    if (ec) { std::cerr << "shinec: " << ec.message() << "\n"; std::exit(1); }


    llvm::legacy::PassManager pass;
    if (tm->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "shinec: can't emit object file for this target\n";
        std::exit(1);
    }
    pass.run(mod);
    dest.flush();
}

static void link(const std::string& objPath, const std::string& exePath) {
#ifdef _WIN32
    std::string cmd = "g++ \"" + objPath + "\" -o \"" + exePath + "\"";
#else
    std::string cmd = "cc \"" + objPath + "\" -o \"" + exePath + "\"";
#endif
    if (std::system(cmd.c_str()) != 0) { std::cerr << "shinec: link failed\n"; std::exit(1); }
}

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "usage: shinec <input.shine> [-o <output>]\n"; return 1; }

    std::string in, out;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "-o" && i + 1 < argc) out = argv[++i];
        else in = a;
    }
    if (in.empty()) { std::cerr << "usage: shinec <input.shine> [-o <output>]\n"; return 1; }
    if (out.empty()) {
        out = stripExt(in);
#ifdef _WIN32
        out += ".exe";
#endif
    }

    try {
        Lexer lex(readFile(in), in);
        Parser parser(lex.tokenize());
        Module mod = parser.parseModule(in);

        CodeGen cg;
        auto ir = cg.generate(mod);

        std::string objPath = stripExt(in) + ".o";
        emitObj(*ir, objPath);
        link(objPath, out);

        std::cout << "shinec: built '" << out << "'\n";
        return 0;
    } catch (const CompileError& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}

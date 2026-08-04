#pragma once
#include <vector>
#include "shine/ast.h"
#include "shine/token.h"

namespace shine {

class Parser {
public:
    explicit Parser(std::vector<Token> toks);
    Module parseModule(std::string file);

private:
    const Token& peek(int off = 0) const;
    const Token& advance();
    bool check(TokenKind k) const;
    bool match(TokenKind k);
    const Token& expect(TokenKind k, const std::string& ctx);
    bool atEnd() const;

    FunctionDecl function();
    Param param();
    TypeRef type();
    StmtPtr stmt();
    StmtPtr returnStmt();
    StmtPtr varDecl();
    StmtPtr assignStmt();
    ExprPtr expr();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr primary();

    [[noreturn]] void err(const Token& t, const std::string& msg) const;

    std::vector<Token> toks_;
    size_t pos_ = 0;
};

}

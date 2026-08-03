#include "shine/parser.h"
#include "shine/error.h"

namespace shine {

Parser::Parser(std::vector<Token> toks) : toks_(std::move(toks)) {}

bool Parser::atEnd() const { return peek().kind == TokenKind::EndOfFile; }

const Token& Parser::peek(int off) const {
    size_t i = pos_ + (size_t)off;
    return i < toks_.size() ? toks_[i] : toks_.back();
}

const Token& Parser::advance() {
    const Token& t = peek();
    if (!atEnd()) pos_++;
    return t;
}

bool Parser::check(TokenKind k) const { return peek().kind == k; }

bool Parser::match(TokenKind k) {
    if (!check(k)) return false;
    advance();
    return true;
}

const Token& Parser::expect(TokenKind k, const std::string& ctx) {
    if (!check(k)) err(peek(), "expected " + std::string(tokenName(k)) + " " + ctx +
                                    ", got " + tokenName(peek().kind));
    return advance();
}

void Parser::err(const Token& t, const std::string& msg) const {
    throw CompileError(t.loc, msg);
}

Module Parser::parseModule(std::string file) {
    Module m{std::move(file), {}};
    while (!atEnd()) m.functions.push_back(function());
    return m;
}

TypeRef Parser::type() {
    if (check(TokenKind::KwInt)) return {"int", advance().loc};
    if (check(TokenKind::KwVoid)) return {"void", advance().loc};
    err(peek(), "expected a type");
}

Param Parser::param() {
    const Token& name = expect(TokenKind::Identifier, "as param name");
    expect(TokenKind::Colon, "after param name");
    return {name.text, type()};
}

FunctionDecl Parser::function() {
    expect(TokenKind::KwFn, "to start function");
    TypeRef retType = type();
    const Token& name = expect(TokenKind::Identifier, "as function name");
    FunctionDecl fn{name.text, {}, retType, {}, name.loc};

    expect(TokenKind::LParen, "after function name");
    if (!check(TokenKind::RParen)) {
        fn.params.push_back(param());
        while (match(TokenKind::Comma)) fn.params.push_back(param());
    }
    expect(TokenKind::RParen, "to close params");

    expect(TokenKind::LBrace, "to start body");
    while (!check(TokenKind::RBrace)) {
        if (atEnd()) err(peek(), "unexpected end of file in function body");
        fn.body.push_back(stmt());
    }
    expect(TokenKind::RBrace, "to close body");
    return fn;
}

StmtPtr Parser::stmt() {
    if (check(TokenKind::KwReturn)) return returnStmt();
    auto s = std::make_unique<ExprStmt>();
    s->loc = peek().loc;
    s->expr = expr();
    expect(TokenKind::Semicolon, "after expression");
    return s;
}

StmtPtr Parser::returnStmt() {
    const Token& kw = advance();
    auto s = std::make_unique<ReturnStmt>();
    s->loc = kw.loc;
    if (!check(TokenKind::Semicolon)) s->value = expr();
    expect(TokenKind::Semicolon, "after r/ statement");
    return s;
}

ExprPtr Parser::expr() { return primary(); }

ExprPtr Parser::primary() {
    if (check(TokenKind::IntLiteral)) {
        const Token& t = advance();
        auto e = std::make_unique<IntLiteralExpr>();
        e->loc = t.loc;
        e->value = t.intValue;
        return e;
    }
    if (check(TokenKind::StringLiteral)) {
        const Token& t = advance();
        auto e = std::make_unique<StringLiteralExpr>();
        e->loc = t.loc;
        e->value = t.text;
        return e;
    }
    if (check(TokenKind::Identifier)) {
        const Token& name = advance();
        std::string callee = name.text;
        while (check(TokenKind::Dot)) {
            advance();
            callee += "." + expect(TokenKind::Identifier, "after '.'").text;
        }
        if (!check(TokenKind::LParen)) {
            if (callee != name.text) err(name, "expected '(' after '" + callee + "'");
            auto e = std::make_unique<IdentifierExpr>();
            e->loc = name.loc;
            e->name = callee;
            return e;
        }
        advance();
        auto call = std::make_unique<CallExpr>();
        call->loc = name.loc;
        call->callee = callee;
        if (!check(TokenKind::RParen)) {
            call->args.push_back(expr());
            while (match(TokenKind::Comma)) call->args.push_back(expr());
        }
        expect(TokenKind::RParen, "to close call");
        return call;
    }
    err(peek(), "expected an expression");
}

}

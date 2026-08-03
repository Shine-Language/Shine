#pragma once
#include <string>
#include <cstdint>

namespace shine {

enum class TokenKind {
    IntLiteral, StringLiteral, Identifier,
    KwFn, KwReturn, KwInt, KwVoid,
    LParen, RParen, LBrace, RBrace, Semicolon, Comma, Colon, Dot, Arrow,
    EndOfFile, Invalid,
};

struct SourceLoc {
    std::string file;
    int line = 1, col = 1;
};

struct Token {
    TokenKind kind = TokenKind::Invalid;
    std::string text;
    int64_t intValue = 0;
    SourceLoc loc;
};

const char* tokenName(TokenKind kind);

}

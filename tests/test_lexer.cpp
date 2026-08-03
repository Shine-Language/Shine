#include "shine/lexer.h"
#include "test_framework.h"

using namespace shine;

TEST(empty_input_is_just_eof) {
    Lexer l("", "t.shine");
    auto toks = l.tokenize();
    ASSERT_EQ(toks.size(), 1u);
    ASSERT_TRUE(toks[0].kind == TokenKind::EndOfFile);
}

TEST(keyword_and_identifier) {
    Lexer l("fn main", "t.shine");
    auto toks = l.tokenize();
    ASSERT_TRUE(toks[0].kind == TokenKind::KwFn);
    ASSERT_TRUE(toks[1].kind == TokenKind::Identifier);
    ASSERT_EQ(toks[1].text, "main");
}

TEST(int_literal_value) {
    Lexer l("42", "t.shine");
    ASSERT_EQ(l.tokenize()[0].intValue, 42);
}

TEST(string_literal_escapes) {
    Lexer l(R"("a\nb")", "t.shine");
    ASSERT_EQ(l.tokenize()[0].text, "a\nb");
}

TEST(comments_skipped) {
    Lexer l("// x\nfn /* y */ main", "t.shine");
    auto toks = l.tokenize();
    ASSERT_TRUE(toks[0].kind == TokenKind::KwFn);
    ASSERT_TRUE(toks[1].kind == TokenKind::Identifier);
}

TEST(arrow_token) {
    Lexer l("->", "t.shine");
    ASSERT_TRUE(l.tokenize()[0].kind == TokenKind::Arrow);
}

TEST(r_slash_is_return_keyword) {
    Lexer l("r/0;", "t.shine");
    auto toks = l.tokenize();
    ASSERT_TRUE(toks[0].kind == TokenKind::KwReturn);
    ASSERT_TRUE(toks[1].kind == TokenKind::IntLiteral);
}

TEST(bare_r_identifier_still_works) {
    Lexer l("r(5)", "t.shine");
    auto toks = l.tokenize();
    ASSERT_TRUE(toks[0].kind == TokenKind::Identifier);
    ASSERT_EQ(toks[0].text, "r");
}

TEST(r_before_comment_is_identifier_not_return) {
    Lexer l("r // comment", "t.shine");
    auto toks = l.tokenize();
    ASSERT_TRUE(toks[0].kind == TokenKind::Identifier);
    ASSERT_EQ(toks[0].text, "r");
}

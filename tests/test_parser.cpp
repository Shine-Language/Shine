#include "shine/error.h"
#include "shine/lexer.h"
#include "shine/parser.h"
#include "test_framework.h"

using namespace shine;

static Module parse(const std::string& src) {
    Lexer l(src, "t.shine");
    Parser p(l.tokenize());
    return p.parseModule("t.shine");
}

TEST(empty_void_function) {
    Module m = parse("fn void main() { }");
    ASSERT_EQ(m.functions.size(), 1u);
    ASSERT_EQ(m.functions[0].name, "main");
}

TEST(return_int_literal) {
    Module m = parse("fn int main() { r/0; }");
    auto* r = dynamic_cast<ReturnStmt*>(m.functions[0].body[0].get());
    ASSERT_TRUE(r != nullptr);
    auto* i = dynamic_cast<IntLiteralExpr*>(r->value.get());
    ASSERT_TRUE(i != nullptr);
    ASSERT_EQ(i->value, 0);
}

TEST(call_statement) {
    Module m = parse(R"(fn void main() { write("hi"); })");
    auto* es = dynamic_cast<ExprStmt*>(m.functions[0].body[0].get());
    auto* c = dynamic_cast<CallExpr*>(es->expr.get());
    ASSERT_TRUE(c != nullptr);
    ASSERT_EQ(c->callee, "write");
}

TEST(let_decl_uses_parenthesized_type) {
    Module m = parse("fn int main() { let(int) a = 10; r/a; }");
    auto* v = dynamic_cast<VarDeclStmt*>(m.functions[0].body[0].get());
    ASSERT_TRUE(v != nullptr);
    ASSERT_TRUE(!v->isMutable);
    ASSERT_EQ(v->type.name, "int");
    ASSERT_EQ(v->name, "a");
}

TEST(var_decl_and_assignment) {
    Module m = parse("fn int main() { var(int) a = 10; a = a + 1; r/a; }");
    auto* v = dynamic_cast<VarDeclStmt*>(m.functions[0].body[0].get());
    auto* a = dynamic_cast<AssignStmt*>(m.functions[0].body[1].get());
    ASSERT_TRUE(v != nullptr);
    ASSERT_TRUE(v->isMutable);
    ASSERT_TRUE(a != nullptr);
    ASSERT_EQ(a->name, "a");
}

TEST(multiply_binds_before_add) {
    Module m = parse("fn int main() { r/1 + 2 * 3; }");
    auto* r = dynamic_cast<ReturnStmt*>(m.functions[0].body[0].get());
    auto* add = dynamic_cast<BinaryExpr*>(r->value.get());
    ASSERT_TRUE(add != nullptr);
    ASSERT_EQ(add->op, "+");
    auto* mul = dynamic_cast<BinaryExpr*>(add->right.get());
    ASSERT_TRUE(mul != nullptr);
    ASSERT_EQ(mul->op, "*");
}

TEST(grouped_expression) {
    Module m = parse("fn int main() { r/(1 + 2) * 3; }");
    auto* r = dynamic_cast<ReturnStmt*>(m.functions[0].body[0].get());
    auto* mul = dynamic_cast<BinaryExpr*>(r->value.get());
    ASSERT_TRUE(mul != nullptr);
    ASSERT_EQ(mul->op, "*");
    auto* add = dynamic_cast<BinaryExpr*>(mul->left.get());
    ASSERT_TRUE(add != nullptr);
    ASSERT_EQ(add->op, "+");
}

TEST(multiple_functions) {
    Module m = parse("fn void a() { } fn void b() { }");
    ASSERT_EQ(m.functions.size(), 2u);
}

TEST(missing_return_type_throws) {
    bool threw = false;
    try { parse("fn main() { }"); }
    catch (const CompileError&) { threw = true; }
    ASSERT_TRUE(threw);
}

TEST(unterminated_block_throws) {
    bool threw = false;
    try { parse("fn void main() {"); }
    catch (const CompileError&) { threw = true; }
    ASSERT_TRUE(threw);
}

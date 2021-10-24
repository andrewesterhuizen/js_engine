#include "catch.hpp"

#include "../lexer.h"
#include "../parser.h"

TEST_CASE("Parser parses number literal", "[parser][ast]") {
    auto source = R"(123;)";

    lexer::Lexer l;
    auto tokens = l.get_tokens(source);

    parser::Parser p;
    auto ast = p.parse(tokens);

    REQUIRE(ast.body.size() == 1);
    REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

    auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
    REQUIRE(expression_statement->expression->type == ast::ExpressionType::NumberLiteral);

    auto expression = std::static_pointer_cast<ast::NumberLiteralExpression>(expression_statement->expression);
    REQUIRE(expression->value == 123);
}
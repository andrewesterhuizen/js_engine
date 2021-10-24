#include "catch.hpp"

#include "../lexer.h"
#include "../parser.h"

ast::Program get_ast(std::string source) {
    lexer::Lexer l;
    auto tokens = l.get_tokens(source);
    parser::Parser p;
    return p.parse(tokens);
}

TEST_CASE("Parser parses literals", "[parser][ast]") {
    SECTION("number literals") {
        auto source = R"(123;)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::NumberLiteral);
        auto expression = std::static_pointer_cast<ast::NumberLiteralExpression>(expression_statement->expression);
        REQUIRE(expression->value == 123);
    }

    SECTION("string literals") {
        auto source = R"("a test string";)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::StringLiteral);
        auto expression = std::static_pointer_cast<ast::StringLiteralExpression>(expression_statement->expression);
        REQUIRE(expression->value == "a test string");
    }

    SECTION("boolean literals: true") {
        auto source = R"(true;)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::BooleanLiteral);
        auto expression = std::static_pointer_cast<ast::BooleanLiteralExpression>(expression_statement->expression);
        REQUIRE(expression->value == true);
    }

    SECTION("boolean literals: false") {
        auto source = R"(false;)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::BooleanLiteral);
        auto expression = std::static_pointer_cast<ast::BooleanLiteralExpression>(expression_statement->expression);
        REQUIRE(expression->value == false);
    }

    SECTION("array literals") {
        auto source = R"([1,2,3];)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::Array);
        auto expression = std::static_pointer_cast<ast::ArrayExpression>(expression_statement->expression);
        REQUIRE(expression->elements.size() == 3);

        for (auto i = 0; i < 3; i++) {
            auto el = expression->elements[i];
            REQUIRE(el->type == ast::ExpressionType::NumberLiteral);
            auto n = std::static_pointer_cast<ast::NumberLiteralExpression>(el);
            REQUIRE(n->value == i + 1);
        }
    }

    SECTION("object literals") {
        auto source = R"(({ x: 123, y: 234});)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::Object);
        auto expression = std::static_pointer_cast<ast::ObjectExpression>(expression_statement->expression);
        REQUIRE(expression->properties.size() == 2);

        auto x = expression->properties.find("x");
        REQUIRE(x != expression->properties.end());
        REQUIRE(x->second->type == ast::ExpressionType::NumberLiteral);
        auto xn = std::static_pointer_cast<ast::NumberLiteralExpression>(x->second);
        REQUIRE(xn->value == 123);

        auto y = expression->properties.find("y");
        REQUIRE(y != expression->properties.end());
        REQUIRE(y->second->type == ast::ExpressionType::NumberLiteral);
        auto yn = std::static_pointer_cast<ast::NumberLiteralExpression>(y->second);
        REQUIRE(yn->value == 234);
    }
}

TEST_CASE("Parser parses expressions", "[parser][ast]") {
    SECTION("expressions in parentheses") {
        auto source = R"((123);)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        REQUIRE(ast.body[0]->type == ast::StatementType::Expression);

        auto expression_statement = std::static_pointer_cast<ast::ExpressionStatement>(ast.body[0]);
        REQUIRE(expression_statement->expression->type == ast::ExpressionType::NumberLiteral);
        auto expression = std::static_pointer_cast<ast::NumberLiteralExpression>(expression_statement->expression);
        REQUIRE(expression->value == 123);
    }
}
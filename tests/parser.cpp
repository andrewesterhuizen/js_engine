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
        auto expression_statement = ast.body[0]->as_expression_statement();

        auto expression = expression_statement->expression->as_number_literal();
        REQUIRE(expression->value == 123);
    }

    SECTION("string literals") {
        auto source = R"("a test string";)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();

        auto expression = expression_statement->expression->as_string_literal();
        REQUIRE(expression->value == "a test string");
    }

    SECTION("boolean literals: true") {
        auto source = R"(true;)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();

        auto expression = expression_statement->expression->as_boolean_literal();
        REQUIRE(expression->value == true);
    }

    SECTION("boolean literals: false") {
        auto source = R"(false;)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();

        auto expression = expression_statement->expression->as_boolean_literal();
        REQUIRE(expression->value == false);
    }

    SECTION("array literals") {
        auto source = R"([1,2,3];)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();

        auto expression = expression_statement->expression->as_array();
        REQUIRE(expression->elements.size() == 3);

        for (auto i = 0; i < 3; i++) {
            auto el = expression->elements[i];
            auto n = el->as_number_literal();
            REQUIRE(n->value == i + 1);
        }
    }

    SECTION("object literals") {
        auto source = R"(({ x: 123, y: 234});)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();

        auto expression = expression_statement->expression->as_object();
        REQUIRE(expression->properties.size() == 2);

        auto x = expression->properties.find("x");
        REQUIRE(x != expression->properties.end());
        auto xn = x->second->as_number_literal();
        REQUIRE(xn->value == 123);

        auto y = expression->properties.find("y");
        REQUIRE(y != expression->properties.end());
        auto yn = y->second->as_number_literal();
        REQUIRE(yn->value == 234);
    }
}

TEST_CASE("Parser parses expressions", "[parser][ast]") {
    SECTION("expressions in parentheses") {
        auto source = R"((123);)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();
        auto expression = expression_statement->expression->as_number_literal();
        REQUIRE(expression->value == 123);
    }

    SECTION("identifiers") {
        auto source = R"(test;)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();
        auto expression = expression_statement->expression->as_identifier();
        REQUIRE(expression->name == "test");
    }

    SECTION("identifiers in parentheses") {
        auto source = R"((test);)";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto expression_statement = ast.body[0]->as_expression_statement();
        auto expression = expression_statement->expression->as_identifier();
        REQUIRE(expression->name == "test");
    }
}


TEST_CASE("Parser parses statements", "[parser][ast]") {
    SECTION("while statement") {
        auto source = R"(
            while(test) {
                123;
            }
        )";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto s = ast.body[0]->as_while();
        auto test = s->test->as_identifier();
        REQUIRE(test->name == "test");
        REQUIRE(s->body->type == ast::StatementType::Block);
    }

    SECTION("return statement") {
        auto source = R"(
            return 123;
        )";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto s = ast.body[0]->as_return();
        auto n = s->argument->as_number_literal();
        REQUIRE(n->value == 123);
    }

    SECTION("return statement without value") {
        auto source = R"(
            return;
        )";
        auto ast = get_ast(source);

        REQUIRE(ast.body.size() == 1);
        auto s = ast.body[0]->as_return();
        REQUIRE(s->argument == nullptr);
    }
}
#include "catch.hpp"

#include "../lexer.h"
#include "../parser.h"

void lexer_test_case(std::string source, std::vector<lexer::Token> expected_tokens) {
    lexer::Lexer l;
    auto tokens = l.get_tokens(source);

    REQUIRE(tokens.size() == expected_tokens.size());

    for (auto i = 0; i < tokens.size(); i++) {
        REQUIRE(tokens[i].type == expected_tokens[i].type);
        REQUIRE(tokens[i].value == expected_tokens[i].value);
    }
}

TEST_CASE("Lexer returns expected tokens", "[lexer][ast]") {
    SECTION("string") {
        auto source = R"("test")";

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::String, "test"}
        };

        lexer_test_case(source, expected);
    }

    SECTION("multiple strings") {
        auto source = R"("test","test2","test3")";

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::String, "test"},
                {lexer::TokenType::Comma, ","},
                {lexer::TokenType::String, "test2"},
                {lexer::TokenType::Comma, ","},
                {lexer::TokenType::String, "test3"},
        };

        lexer_test_case(source, expected);
    }

    SECTION("arrow function") {
        auto source = R"(() => 1;)";

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::LeftParen,  "("},
                {lexer::TokenType::RightParen, ")"},
                {lexer::TokenType::Arrow,      "=>"},
                {lexer::TokenType::Number,     "1"},
                {lexer::TokenType::Semicolon,  ";"},
        };

        lexer_test_case(source, expected);
    }

    SECTION("arrow function with body") {
        auto source = R"(() => { return 1; })";

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::LeftParen,  "("},
                {lexer::TokenType::RightParen, ")"},
                {lexer::TokenType::Arrow,      "=>"},
                {lexer::TokenType::LeftBrace,  "{"},
                {lexer::TokenType::Keyword,    "return"},
                {lexer::TokenType::Number,     "1"},
                {lexer::TokenType::Semicolon,  ";"},
                {lexer::TokenType::RightBrace, "}"},
        };

        lexer_test_case(source, expected);
    }

    SECTION("arrow function with parameters") {
        auto source = R"((a,b) => 1;)";

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::LeftParen,  "("},
                {lexer::TokenType::Identifier, "a"},
                {lexer::TokenType::Comma,      ","},
                {lexer::TokenType::Identifier, "b"},
                {lexer::TokenType::RightParen, ")"},
                {lexer::TokenType::Arrow,      "=>"},
                {lexer::TokenType::Number,     "1"},
                {lexer::TokenType::Semicolon,  ";"},
        };

        lexer_test_case(source, expected);
    }

}

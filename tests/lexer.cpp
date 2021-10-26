#include "catch.hpp"

#include "../lexer.h"
#include "../parser.h"

std::vector<lexer::Token> get_tokens(std::string source) {
    lexer::Lexer l;
    return l.get_tokens(source);
}

TEST_CASE("Lexer returns expected tokens", "[lexer][ast]") {
    SECTION("string") {
        auto source = R"("test")";
        auto tokens = get_tokens(source);

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::String, "test"}
        };

        REQUIRE(tokens.size() == expected.size());

        for (auto i = 0; i < tokens.size(); i++) {
            REQUIRE(tokens[i].type == expected[i].type);
            REQUIRE(tokens[i].value == expected[i].value);
        }
    }

    SECTION("multiple strings") {
        auto source = R"("test","test2","test3")";
        auto tokens = get_tokens(source);

        std::vector<lexer::Token> expected = {
                {lexer::TokenType::String, "test"},
                {lexer::TokenType::Comma, ","},
                {lexer::TokenType::String, "test2"},
                {lexer::TokenType::Comma, ","},
                {lexer::TokenType::String, "test3"},
        };

        REQUIRE(tokens.size() == expected.size());

        for (auto i = 0; i < tokens.size(); i++) {
            REQUIRE(tokens[i].type == expected[i].type);
            REQUIRE(tokens[i].value == expected[i].value);
        }
    }

}

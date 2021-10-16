#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include "json.hpp"

namespace lexer {

enum class TokenType {
    Keyword,
    Identifier,
    Number,
    String,
    Equals,
    Plus,
    Semicolon,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Dot,
    EndOfFile
};

std::string token_type_to_string(TokenType type);

struct Token {
    TokenType type;
    std::string value;

    nlohmann::json to_json() {
        nlohmann::json j;
        j["type"] = token_type_to_string(type);
        j["value"] = value;
        return j;
    }
};

class Lexer {
    std::vector<Token> tokens;
    int index = 0;
    std::string source;

    std::unordered_map<char, TokenType> single_char_tokens = {
            {'\0', TokenType::EndOfFile},
            {';',  TokenType::Semicolon},
            {'=',  TokenType::Equals},
            {'+',  TokenType::Plus},
            {'(',  TokenType::LeftParen},
            {')',  TokenType::RightParen},
            {'{',  TokenType::LeftBrace},
            {'}',  TokenType::RightBrace},
            {'.',  TokenType::Dot},
    };
    std::unordered_set<std::string> keywords = {"var", "if", "else"};

    void emit_token(TokenType type, std::string value) {
        tokens.push_back(Token{type, value});
    }

    char next_char() {
        if (index + 1 > source.length()) {
            return 0;
        }

        return source[++index];
    }

    std::string get_text_until_whitespace() {
        std::string text;
        auto c = source[index];
        while (c != ' ' && c != '\n') {
            text += c;
            c = next_char();

            if (c == '\0') break;
        }

        return text;
    }

    bool is_single_char_token(char c) {
        return single_char_tokens.find(c) != single_char_tokens.end();
    }

    std::string get_text_until_next_token_or_whitespace() {
        std::string text;
        auto c = source[index];
        while (!is_single_char_token(c) && c != ' ' && c != '\n') {
            text += c;
            c = next_char();

            if (c == '\0') break;
        }

        return text;
    }

    std::string get_string(char quote_type) {
        std::string text;
        auto c = source[index];
        while (c != quote_type && c != '\n') {
            text += c;
            c = next_char();

            if (c == '\0') break;
        }

        return text;
    }

    void skip_whitespace() {
        auto c = source[index];
        while (c == ' ' || c == '\n') {
            c = next_char();

            if (c == '\0') break;
        }
    }

    void get_number() {
        std::string text;

        bool seen_decimal_point = false;
        auto c = source[index];

        while (isdigit(c) || (!seen_decimal_point && c == '.')) {
            if (c == '.') {
                seen_decimal_point = true;
            }

            text += c;
            c = next_char();

            if (c == '\0') break;
        }

        emit_token(TokenType::Number, text);
    }

    void get_token() {
        skip_whitespace();

        auto c = source[index];


        if (auto entry = single_char_tokens.find(c); entry != single_char_tokens.end()) {
            emit_token(entry->second, std::string(1, entry->first));
            next_char();
        } else if (c == '"') {
            next_char();
            auto text = get_string(c);
            emit_token(TokenType::String, text);
            next_char();
        } else if (isdigit(c)) {
            get_number();
        } else if (isalnum(c)) {
            auto text = get_text_until_next_token_or_whitespace();
            auto is_keyword = keywords.find(text) != keywords.end();
            emit_token(is_keyword ? TokenType::Keyword : TokenType::Identifier, text);
        } else {
            std::cerr << "unexpected token: " << c << "\n";
            assert(false);
        }
    }

public:
    std::vector<Token> get_tokens(std::string src) {
        source = src;
        index = 0;

        while (index < source.length()) {
            get_token();
        }

        return tokens;
    }
};

}
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
    Minus,
    Slash,
    Asterisk,
    Percent,
    Semicolon,
    Colon,
    Comma,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Dot,
    EndOfFile
};

std::string token_type_to_string(TokenType type);

struct Token {
    TokenType type;
    std::string value;
    nlohmann::json to_json();
};

class Lexer {
    std::vector<Token> tokens;
    int index = 0;
    std::string source;

    std::unordered_map<char, TokenType> single_char_tokens = {
            {'\0', TokenType::EndOfFile},
            {';',  TokenType::Semicolon},
            {':',  TokenType::Colon},
            {',',  TokenType::Comma},
            {'=',  TokenType::Equals},
            {'+',  TokenType::Plus},
            {'-',  TokenType::Minus},
            {'*',  TokenType::Asterisk},
            {'/',  TokenType::Slash},
            {'%',  TokenType::Percent},
            {'(',  TokenType::LeftParen},
            {')',  TokenType::RightParen},
            {'{',  TokenType::LeftBrace},
            {'}',  TokenType::RightBrace},
            {'[',  TokenType::LeftBracket},
            {']',  TokenType::RightBracket},
            {'.',  TokenType::Dot},
    };
    std::unordered_set<std::string> keywords = {"var", "if", "else", "function"};

    void emit_token(TokenType type, std::string value);
    char next_char();
    bool is_single_char_token(char c);
    std::string get_text_until_next_token_or_whitespace();
    void skip_whitespace();
    std::string get_string(char quote_type);
    void get_number();
    void get_token();
public:
    std::vector<Token> get_tokens(std::string src);
};

}
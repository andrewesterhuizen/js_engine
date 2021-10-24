#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <regex>

#include "json.hpp"

namespace lexer {

enum class TokenType {
    Keyword,
    Identifier,
    Number,
    String,
    Plus,
    Minus,
    Slash,
    Asterisk,
    Percent,
    Increment,
    Decrement,
    Exponentiation,
    AdditionAssignment,
    SubtractionAssignment,
    MultiplicationAssignment,
    DivisionAssignment,
    And,
    Or,
    EqualTo,
    EqualToStrict,
    NotEqualTo,
    NotEqualToStrict,
    LessThan,
    LessThanOrEqualTo,
    GreaterThan,
    GreaterThanOrEqualTo,
    Equals,
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
    QuestionMark,
    NewLine,
    EndOfFile
};

std::string token_type_to_string(TokenType type);

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    nlohmann::json to_json();
};

struct Pattern {
    std::regex pattern;
    TokenType token_type;
    Pattern(std::string pattern, TokenType token_type) : pattern(pattern), token_type(token_type) {}
};

class Lexer {
    std::vector<Token> tokens;
    int index = 0;
    std::string source;

    int line = 1;
    int column = 0;

    const std::string keywords_regex = "^(var|if|else|function|true|false|while|for|return|let|const)";

    std::vector<Pattern> patterns = {
            {keywords_regex,          TokenType::Keyword},
            {"^(_|\\$|[a-zA-Z])\\w*", TokenType::Identifier},
            {"^\".*\"",               TokenType::String},
            {"^\\d[.\\d+]*",          TokenType::Number},
            {"^===",                  TokenType::EqualToStrict},
            {"^==",                   TokenType::EqualTo},
            {"^=",                    TokenType::Equals},
            {"^>=",                   TokenType::GreaterThanOrEqualTo},
            {"^>",                    TokenType::GreaterThan},
            {"^<=",                   TokenType::LessThanOrEqualTo},
            {"^<",                    TokenType::LessThan},
            {"^&&",                   TokenType::And},
            {"^\\|\\|",               TokenType::Or},
            {"^!==",                  TokenType::NotEqualToStrict},
            {"^!=",                   TokenType::NotEqualTo},
            {"^\\+=",                 TokenType::AdditionAssignment},
            {"^\\+\\+",               TokenType::Increment},
            {"^\\+",                  TokenType::Plus},
            {"^-=",                   TokenType::SubtractionAssignment},
            {"^--",                   TokenType::Decrement},
            {"^-",                    TokenType::Minus},
            {"^;",                    TokenType::Semicolon},
            {"^:",                    TokenType::Colon},
            {"^,",                    TokenType::Comma},
            {"^\\*=",                 TokenType::MultiplicationAssignment},
            {"^\\*\\*",               TokenType::Exponentiation},
            {"^\\*",                  TokenType::Asterisk},
            {"^/=",                   TokenType::DivisionAssignment},
            {"^/",                    TokenType::Slash},
            {"^%",                    TokenType::Percent},
            {"^\\(",                  TokenType::LeftParen},
            {"^\\)",                  TokenType::RightParen},
            {"^\\{",                  TokenType::LeftBrace},
            {"^\\}",                  TokenType::RightBrace},
            {"^\\[",                  TokenType::LeftBracket},
            {"^\\]",                  TokenType::RightBracket},
            {"^\\.",                  TokenType::Dot},
            {"^\\?",                  TokenType::QuestionMark},
            {"^\n",                   TokenType::NewLine},
            {"^\\0",                  TokenType::EndOfFile},
    };

    void emit_token(TokenType type, std::string value);
    char next_char();
    std::string get_rest_of_line();
    void skip_whitespace();
    void get_token();
public:
    std::vector<Token> get_tokens(std::string src);
};

}
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <regex>

#include "json.hpp"

namespace lexer {

#define TOKENS(MAP) \
    MAP(EndOfFile) \
    MAP(Keyword) \
    MAP(Identifier) \
    MAP(Number) \
    MAP(String) \
    MAP(Plus) \
    MAP(Minus) \
    MAP(Slash) \
    MAP(Asterisk) \
    MAP(Percent) \
    MAP(Increment) \
    MAP(Decrement) \
    MAP(Exponentiation) \
    MAP(AdditionAssignment) \
    MAP(SubtractionAssignment) \
    MAP(MultiplicationAssignment) \
    MAP(DivisionAssignment) \
    MAP(And) \
    MAP(Or) \
    MAP(EqualTo) \
    MAP(EqualToStrict) \
    MAP(NotEqualTo) \
    MAP(NotEqualToStrict)   \
    MAP(Not) \
    MAP(LessThan) \
    MAP(LessThanOrEqualTo) \
    MAP(GreaterThan) \
    MAP(GreaterThanOrEqualTo) \
    MAP(Equals) \
    MAP(Semicolon) \
    MAP(Colon) \
    MAP(Comma) \
    MAP(LeftParen) \
    MAP(RightParen) \
    MAP(LeftBrace) \
    MAP(RightBrace) \
    MAP(LeftBracket) \
    MAP(RightBracket) \
    MAP(Dot) \
    MAP(QuestionMark) \
    MAP(Arrow) \
    MAP(NewLine) \
    MAP(Pipe) \
    MAP(Ampersand)

#define CREATE_ENUM(NAME) NAME,

enum class TokenType {
    TOKENS(CREATE_ENUM)
};

#undef CREATE_ENUM

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

    const std::string keywords_regex = "^("
                                       "break|"
                                       "case|"
                                       "catch|"
                                       "class|"
                                       "const|"
                                       "continue|"
                                       "debugger|"
                                       "default|"
                                       "delete|"
                                       "do|"
                                       "else|"
                                       "export|"
                                       "extends|"
                                       "false|"
                                       "finally|"
                                       "for|"
                                       "function|"
                                       "if|"
                                       "import|"
                                       "in|"
                                       "instanceof|"
                                       "let|"
                                       "new|"
                                       "null|"
                                       "return|"
                                       "super|"
                                       "switch|"
                                       "this|"
                                       "throw|"
                                       "true|"
                                       "try|"
                                       "typeof|"
                                       "var|"
                                       "void|"
                                       "while|"
                                       "with|"
                                       "yield)"
                                       "(?!\\w)";

    std::vector<Pattern> patterns = {
            {keywords_regex,          TokenType::Keyword},
            {"^(_|\\$|[a-zA-Z])\\w*", TokenType::Identifier},
            {"^\"[^\"]*\"",           TokenType::String},
            {"^'[^']*'",              TokenType::String},
            {"^0[xX][0-9a-fA-F]+",    TokenType::Number},
            {"^\\d[.\\d+]*",          TokenType::Number},
            {"^=>",                   TokenType::Arrow},
            {"^===",                  TokenType::EqualToStrict},
            {"^==",                   TokenType::EqualTo},
            {"^=",                    TokenType::Equals},
            {"^>=",                   TokenType::GreaterThanOrEqualTo},
            {"^>",                    TokenType::GreaterThan},
            {"^<=",                   TokenType::LessThanOrEqualTo},
            {"^<",                    TokenType::LessThan},
            {"^&&",                   TokenType::And},
            {"^&",                    TokenType::Ampersand},
            {"^\\|\\|",               TokenType::Or},
            {"^\\|",                  TokenType::Pipe},
            {"^!==",                  TokenType::NotEqualToStrict},
            {"^!=",                   TokenType::NotEqualTo},
            {"^!",                    TokenType::Not},
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
    void skip_multi_line_comment();
    void get_token();
public:
    std::vector<Token> get_tokens(std::string src);
};

}
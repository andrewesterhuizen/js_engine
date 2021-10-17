#include "lexer.h"

namespace lexer {

std::string token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::Keyword:
            return "Keyword";
        case TokenType::Identifier:
            return "Identifier";
        case TokenType::String:
            return "String";
        case TokenType::Number:
            return "Number";
        case TokenType::Equals:
            return "Equals";
        case TokenType::Plus:
            return "Plus";
        case TokenType::Semicolon:
            return "Semicolon";
        case TokenType::Colon:
            return "Colon";
        case TokenType::Comma:
            return "Comma";
        case TokenType::LeftParen:
            return "LeftParen";
        case TokenType::RightParen:
            return "RightParen";
        case TokenType::LeftBrace:
            return "LeftBrace";
        case TokenType::RightBrace:
            return "RightBrace";
        case TokenType::Dot:
            return "Dot";
        case TokenType::EndOfFile:
            return "EndOfFile";
    }

    std::cout << "missing string for TokenType\n";
    assert(false);
}

nlohmann::json Token::to_json() {
    nlohmann::json j;
    j["type"] = token_type_to_string(type);
    j["value"] = value;
    return j;
}

void Lexer::emit_token(TokenType type, std::string value) {
    tokens.push_back(Token{type, value});
}

char Lexer::next_char() {
    if (index + 1 > source.length()) {
        return 0;
    }

    return source[++index];
}

bool Lexer::is_single_char_token(char c) {
    return single_char_tokens.find(c) != single_char_tokens.end();
}

std::string Lexer::get_text_until_next_token_or_whitespace() {
    std::string text;
    auto c = source[index];
    while (!is_single_char_token(c) && c != ' ' && c != '\n') {
        text += c;
        c = next_char();

        if (c == '\0') break;
    }

    return text;
}

void Lexer::skip_whitespace() {
    auto c = source[index];
    while (c == ' ' || c == '\n') {
        c = next_char();

        if (c == '\0') break;
    }
}

std::string Lexer::get_string(char quote_type) {
    std::string text;
    auto c = source[index];
    while (c != quote_type && c != '\n') {
        text += c;
        c = next_char();

        if (c == '\0') break;
    }

    return text;
}

void Lexer::get_number() {
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

void Lexer::get_token() {
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

std::vector<Token> Lexer::get_tokens(std::string src) {
    source = src;
    index = 0;

    while (index < source.length()) {
        get_token();
    }

    return tokens;
}

}

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
        case TokenType::EqualTo:
            return "EqualTo";
        case TokenType::EqualToStrict:
            return "EqualToStrict";
        case TokenType::NotEqualToStrict:
            return "NotEqualToStrict";
        case TokenType::And:
            return "And";
        case TokenType::Or:
            return "Or";
        case TokenType::LessThan:
            return "LessThan";
        case TokenType::LessThanOrEqualTo:
            return "LessThanOrEqualTo";
        case TokenType::GreaterThan:
            return "GreaterThan";
        case TokenType::GreaterThanOrEqualTo:
            return "GreaterThanOrEqualTo";
        case TokenType::NotEqualTo:
            return "NotEqualTo";
        case TokenType::Plus:
            return "Plus";
        case TokenType::Minus:
            return "Minus";
        case TokenType::Slash:
            return "Slash";
        case TokenType::Asterisk:
            return "Asterisk";
        case TokenType::Exponentiation:
            return "Exponentiation";
        case TokenType::Percent:
            return "Percent";
        case TokenType::Increment:
            return "Increment";
        case TokenType::AdditionAssignment:
            return "AdditionAssignment";
        case TokenType::SubtractionAssignment:
            return "SubtractionAssignment";
        case TokenType::MultiplicationAssignment:
            return "MultiplicationAssignment";
        case TokenType::DivisionAssignment:
            return "DivisionAssignment";
        case TokenType::Decrement:
            return "Decrement";
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
        case TokenType::LeftBracket:
            return "LeftBracket";
        case TokenType::RightBracket:
            return "RightBracket";
        case TokenType::Dot:
            return "Dot";
        case TokenType::QuestionMark:
            return "QuestionMark";
        case TokenType::EndOfFile:
            return "EndOfFile";
        case TokenType::NewLine:
            return "NewLine";
    }

    std::cout << "missing string for TokenType\n";
    assert(false);
}

nlohmann::json Token::to_json() {
    nlohmann::json j;
    j["type"] = token_type_to_string(type);
    j["value"] = value;
    j["line"] = line;
    j["column"] = column;
    return j;
}

void Lexer::emit_token(TokenType type, std::string value) {
    tokens.push_back(Token{type, value, line, column});
}

char Lexer::next_char() {
    if (index + 1 > source.length()) {
        return 0;
    }

    return source[++index];
}

std::string Lexer::get_rest_of_line() {
    std::string text;

    auto i = index;

    auto c = source[i];
    while (c != '\n') {
        text += c;
        i++;
        c = source[i];

        if (c == '\0') break;
    }

    return text;
}

void Lexer::skip_whitespace() {
    auto c = source[index];
    while (c == ' ' || c == '\n') {
        column++;
        if (c == '\n') {
            line++;
            column = 0;
        }

        c = next_char();
        if (c == '\0') break;
    }
}

void Lexer::get_token() {
    skip_whitespace();

    auto text = get_rest_of_line();

    if (text.starts_with("//")) {
        index += text.length();
        line++;
        column = 0;
        return;
    }

    std::smatch match;

    for (auto p: patterns) {
        auto is_match = std::regex_search(text, match, p.pattern);
        if (is_match) {
            auto matched_text = match.str();
            auto match_length = matched_text.length();

            if (p.token_type == TokenType::String) {
                matched_text = matched_text.substr(1, match_length - 2);
            }

            column += match_length;

            emit_token(p.token_type, matched_text);

            if (p.token_type == TokenType::NewLine) {
                line++;
                column = 0;
            }

            index += match_length;
            return;
        }
    }

    std::cerr << "unexpected token: " << text << " at " << line << ":" << column << "\n";
    assert(false);
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

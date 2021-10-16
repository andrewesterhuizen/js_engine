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

}

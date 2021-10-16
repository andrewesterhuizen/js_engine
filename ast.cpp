#include "ast.h"

#include <iostream>

namespace ast {

std::string operator_to_string(Operator op) {
    switch (op) {
        case Operator::Plus:
            return "+";
    }

    std::cerr << "missing case for Operator in operator_to_string";
    assert(false);
}

Operator token_type_to_operator(lexer::TokenType token_type) {
    switch (token_type) {
        case lexer::TokenType::Plus:
            return Operator::Plus;
        default:
            std::cerr << "missing case for TokenType in token_type_to_operator";
            assert(false);
    }
}


}
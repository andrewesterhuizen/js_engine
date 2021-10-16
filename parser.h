#pragma once

#include <iostream>
#include <vector>

#include "ast.h"
#include "lexer.h"

namespace parser {

class Parser {
    int index = 0;
    std::vector<lexer::Token> tokens;

    void backup();
    lexer::Token next_token();
    lexer::Token peek_next_token();
    lexer::Token expect_next_token(lexer::TokenType type);
    void unexpected_token();

    std::shared_ptr<ast::Expression> parse_expression();
    std::shared_ptr<ast::Statement> parse_statement();
    std::vector<std::shared_ptr<ast::Statement>> parse_statements();

public:
    ast::Program parse(std::vector<lexer::Token> input_tokens);
};

}
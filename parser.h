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
    bool next_token_type_is_end_of_expression();
    bool token_type_is_end_of_expression(lexer::TokenType type);
    bool token_type_is_assignment_operator(lexer::TokenType type);

    std::shared_ptr<ast::Expression> parse_assignment_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_call_expression(std::shared_ptr<ast::Expression> callee);
    std::shared_ptr<ast::Expression> parse_member_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_binary_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_expression_recurse(std::shared_ptr<ast::Expression> left); // TODO: this logic should be merged into parse_expression
    std::shared_ptr<ast::Expression> parse_expression();
    std::shared_ptr<ast::Statement> parse_statement();
    std::vector<std::shared_ptr<ast::Statement>> parse_statements();

public:
    ast::Program parse(std::vector<lexer::Token> input_tokens);
};

}
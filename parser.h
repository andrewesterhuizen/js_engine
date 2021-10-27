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
    void skip_token_if_type(lexer::TokenType type);
    lexer::Token peek_next_token_after_type(lexer::TokenType type);
    void unexpected_token();

    std::shared_ptr<ast::Expression> parse_assignment_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_variable_declaration_expression();
    std::shared_ptr<ast::Expression> parse_array_expression();
    std::shared_ptr<ast::Expression> parse_object_expression();
    std::shared_ptr<ast::Expression> parse_function_expression();
    std::shared_ptr<ast::Expression> parse_arrow_function_expression();
    std::shared_ptr<ast::Expression> parse_update_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_call_expression(std::shared_ptr<ast::Expression> callee);
    std::shared_ptr<ast::Expression> parse_new_expression();
    std::shared_ptr<ast::Expression> parse_member_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_binary_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_ternary_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Expression> parse_expression(std::shared_ptr<ast::Expression> left);
    std::shared_ptr<ast::Statement> parse_statement();
    std::vector<std::shared_ptr<ast::Statement>> parse_statements();

public:
    ast::Program parse(std::vector<lexer::Token> input_tokens);
};

}
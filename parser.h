#pragma once

#include <iostream>
#include <vector>

#include "ast.h"
#include "lexer.h"

namespace parser {

class Parser {
    int index = 0;
    std::vector<lexer::Token> tokens;

    lexer::Token next_token() {
        if (tokens[index].type == lexer::TokenType::EndOfFile) {
            return tokens[index];
        }

        return tokens[++index];
    }

    void backup() {
        index--;
    }

    lexer::Token expect_next_token(lexer::TokenType type) {
        if (tokens[index].type == lexer::TokenType::EndOfFile) {
            return tokens[index];
        }

        auto t = tokens[++index];

        if (t.type != type) {
            unexpected_token();
            assert(false);
        }

        return t;
    }

    void unexpected_token() {
        auto t = tokens[index];
        std::cerr << "unexpected token " << t.to_json().dump(4) << "\n";
    }

    std::shared_ptr<ast::Expression> parse_expression() {
        auto t = tokens[index];

        switch (t.type) {
            case lexer::TokenType::Identifier: {
                auto left = std::make_shared<ast::IdentifierExpression>(t.value);

                auto next = next_token();
                if (next.type == lexer::TokenType::Semicolon || next.type == lexer::TokenType::RightParen) {
                    backup();
                    return left;
                }

                assert(next.type == lexer::TokenType::Dot);

                auto right_token = expect_next_token(lexer::TokenType::Identifier);
                auto right = std::make_shared<ast::IdentifierExpression>(right_token.value);

                auto member_expression = std::make_shared<ast::MemberExpression>();
                member_expression->object = left;
                member_expression->property = right;

                next = next_token();
                if (next.type == lexer::TokenType::Semicolon) {
                    return member_expression;
                }

                assert(next.type == lexer::TokenType::LeftParen);
                next_token();
                auto arg = parse_expression();
                next = next_token();
                assert(next.type == lexer::TokenType::RightParen);

                auto call_expression = std::make_shared<ast::CallExpression>();
                call_expression->callee = member_expression;
                call_expression->arguments.push_back(arg);

                expect_next_token(lexer::TokenType::Semicolon);

                return call_expression;
            }
            default:
                unexpected_token();
                assert(false);
        }
    }

    std::shared_ptr<ast::Statement> parse_statement() {
        auto t = tokens[index];

        switch (t.type) {
            case lexer::TokenType::Keyword: {
                assert(t.value == "var");

                auto s = std::make_shared<ast::VariableDeclarationStatement>();

                auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
                expect_next_token(lexer::TokenType::Equals);

                auto value_token = next_token();
                assert(value_token.type == lexer::TokenType::String);

                // this will break when an expression is assigned to a variable
                expect_next_token(lexer::TokenType::Semicolon);

                s->identifier = std::make_shared<ast::IdentifierExpression>(identifier_token.value);
                s->value = std::make_shared<ast::StringLiteralExpression>(value_token.value);

                return s;
            }
            case lexer::TokenType::Identifier: {
                auto s = std::make_shared<ast::ExpressionStatement>();
                s->expression = parse_expression();
                return s;
            }
            default:
                unexpected_token();
                assert(false);
        }
    }

public:
    ast::Program parse(std::vector<lexer::Token> input_tokens) {
        tokens = input_tokens;
        ast::Program program;

        auto t = tokens[index];

        while (t.type != lexer::TokenType::EndOfFile) {
            switch (t.type) {
                case lexer::TokenType::Keyword:
                case lexer::TokenType::Identifier:
                    program.body.push_back(parse_statement());
                    break;
                case lexer::TokenType::EndOfFile:
                    break;
                default:
                    unexpected_token();
                    assert(false);
            }

            t = next_token();
        }

        return program;
    }
};

}
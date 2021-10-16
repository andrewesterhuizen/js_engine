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
            std::cerr << "unexpected token: expected "
                      << lexer::token_type_to_string(type)
                      << " and got "
                      << lexer::token_type_to_string(t.type)
                      << "\n";
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
            case lexer::TokenType::Number: {
                auto left = std::make_shared<ast::NumberLiteralExpression>(std::stod(t.value));

                auto next = next_token();
                if (next.type == lexer::TokenType::Semicolon || next.type == lexer::TokenType::RightParen) {
                    backup();
                    return left;
                }

                assert(next.type == lexer::TokenType::Plus);
                auto op = ast::token_type_to_operator(lexer::TokenType::Plus);

                next_token();
                auto right = parse_expression();

                return std::make_shared<ast::BinaryExpression>(left, right, op);
            }
            case lexer::TokenType::String: {
                auto left = std::make_shared<ast::StringLiteralExpression>(t.value);

                auto next = next_token();
                if (next.type == lexer::TokenType::Semicolon || next.type == lexer::TokenType::RightParen) {
                    backup();
                    return left;
                }

                assert(false);
            }
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
                if (t.value == "var") {
                    auto s = std::make_shared<ast::VariableDeclarationStatement>();

                    auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
                    expect_next_token(lexer::TokenType::Equals);

                    next_token();
                    auto value = parse_expression();

                    expect_next_token(lexer::TokenType::Semicolon);

                    s->identifier = std::make_shared<ast::IdentifierExpression>(identifier_token.value);
                    s->value = value;

                    return s;
                } else if (t.value == "if") {
                    auto s = std::make_shared<ast::IfStatement>();
                    s->alternative = nullptr;

                    expect_next_token(lexer::TokenType::LeftParen);

                    next_token();
                    s->test = parse_expression();

                    expect_next_token(lexer::TokenType::RightParen);

                    next_token();
                    s->consequent = parse_statement();

                    auto next = next_token();
                    if (next.type == lexer::TokenType::Keyword && next.value == "else") {
                        next_token();
                        s->alternative = parse_statement();
                    }

                    return s;
                }

                assert(false);
            }
            case lexer::TokenType::Identifier: {
                auto s = std::make_shared<ast::ExpressionStatement>();
                s->expression = parse_expression();
                return s;
            }
            case lexer::TokenType::LeftBrace: {
                auto s = std::make_shared<ast::BlockStatement>();
                s->body = parse_statements();
                expect_next_token(lexer::TokenType::RightBrace);
                return s;
            }
            default:
                unexpected_token();
                assert(false);
        }
    }

    std::vector<std::shared_ptr<ast::Statement>> parse_statements() {
        std::vector<std::shared_ptr<ast::Statement>> statements;

        auto t = tokens[index];

        while (t.type != lexer::TokenType::EndOfFile) {
            switch (t.type) {
                case lexer::TokenType::Keyword:
                case lexer::TokenType::Identifier:
                    statements.push_back(parse_statement());
                    break;
                case lexer::TokenType::RightBrace:
                    backup();
                    return statements;
                case lexer::TokenType::LeftBrace:
                case lexer::TokenType::EndOfFile:
                    break;
                default:
                    unexpected_token();
                    assert(false);
            }

            t = next_token();
        }

        return statements;
    }

public:
    ast::Program parse(std::vector<lexer::Token> input_tokens) {
        tokens = input_tokens;
        ast::Program program;
        program.body = parse_statements();
        return program;
    }
};

}
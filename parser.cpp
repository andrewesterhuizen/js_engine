#include "parser.h"

namespace parser {

lexer::Token Parser::next_token() {
    if (tokens[index].type == lexer::TokenType::EndOfFile) {
        return tokens[index];
    }

    return tokens[++index];
}

lexer::Token Parser::peek_next_token() {
    if (tokens[index].type == lexer::TokenType::EndOfFile) {
        return tokens[index];
    }

    return tokens[index + 1];
}

void Parser::backup() {
    index--;
}

lexer::Token Parser::expect_next_token(lexer::TokenType type) {
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

void Parser::unexpected_token() {
    auto t = tokens[index];
    std::cerr << "unexpected token " << t.to_json().dump(4) << "\n";
}

bool Parser::token_type_is_end_of_expression(lexer::TokenType type) {
    return type == lexer::TokenType::Semicolon ||
           type == lexer::TokenType::RightParen ||
           type == lexer::TokenType::RightBrace ||
           type == lexer::TokenType::Comma;
}

std::shared_ptr<ast::Expression> Parser::parse_expression() {
    auto t = tokens[index];

    switch (t.type) {
        case lexer::TokenType::Number: {
            auto left = std::make_shared<ast::NumberLiteralExpression>(std::stod(t.value));

            auto next = next_token();
            if (token_type_is_end_of_expression(next.type)) {
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
            if (token_type_is_end_of_expression(next.type)) {
                backup();
                return left;
            }

            assert(false);
        }
        case lexer::TokenType::Identifier: {
            std::shared_ptr<ast::Expression> left = std::make_shared<ast::IdentifierExpression>(t.value);

            auto next = next_token();
            if (next.type == lexer::TokenType::Semicolon || next.type == lexer::TokenType::RightParen) {
                backup();
                return left;
            }

            // member expression
            if (next.type == lexer::TokenType::Dot) {
                auto right_token = expect_next_token(lexer::TokenType::Identifier);
                auto right = std::make_shared<ast::IdentifierExpression>(right_token.value);

                auto member_expression = std::make_shared<ast::MemberExpression>(left, right);

                next = next_token();
                if (next.type == lexer::TokenType::Semicolon) {
                    return member_expression;
                }

                if (next.type == lexer::TokenType::RightParen) {
                    backup();
                    return member_expression;
                }

                left = member_expression;
            }

            // assignment
            if (next.type == lexer::TokenType::Equals) {
                next_token();
                auto right = parse_expression();
                auto ae = std::make_shared<ast::AssignmentExpression>(left, right, ast::Operator::Equals);
                expect_next_token(lexer::TokenType::Semicolon);
                return ae;
            }

            // call
            auto call_expression = std::make_shared<ast::CallExpression>(left);

            assert(next.type == lexer::TokenType::LeftParen);
            next = peek_next_token();
            if (next.type != lexer::TokenType::RightParen) {
                next_token();
                auto arg = parse_expression();
                call_expression->arguments.push_back(arg);
            }

            next = next_token();
            assert(next.type == lexer::TokenType::RightParen);
            expect_next_token(lexer::TokenType::Semicolon);

            return call_expression;
        }
        case lexer::TokenType::LeftBrace: {
            auto next = next_token();
            auto oe = std::make_shared<ast::ObjectExpression>();

            // get properties
            while (next.type != lexer::TokenType::RightBrace) {
                auto id = next;
                assert(id.type == lexer::TokenType::Identifier);
                
                expect_next_token(lexer::TokenType::Colon);

                next_token();
                auto value = parse_expression();
                oe->properties[id.value] = value;
                
                next = next_token();
                if (next.type == lexer::TokenType::Comma) {
                    next = next_token();
                }
            }

            return oe;
        }
        default:
            unexpected_token();
            assert(false);
    }
}

std::shared_ptr<ast::Statement> Parser::parse_statement() {
    auto t = tokens[index];

    switch (t.type) {
        case lexer::TokenType::Keyword: {
            if (t.value == "var") {
                auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
                expect_next_token(lexer::TokenType::Equals);

                next_token();
                auto value = parse_expression();

                expect_next_token(lexer::TokenType::Semicolon);

                return std::make_shared<ast::VariableDeclarationStatement>(identifier_token.value, value);
            } else if (t.value == "if") {
                std::shared_ptr<ast::Statement> alternative = nullptr;

                expect_next_token(lexer::TokenType::LeftParen);

                next_token();
                auto test = parse_expression();

                expect_next_token(lexer::TokenType::RightParen);

                next_token();
                auto consequent = parse_statement();

                auto next = next_token();
                if (next.type == lexer::TokenType::Keyword && next.value == "else") {
                    next_token();
                    alternative = parse_statement();
                }

                return std::make_shared<ast::IfStatement>(test, consequent, alternative);
            } else if (t.value == "function") {
                auto identifier_token = expect_next_token(lexer::TokenType::Identifier);

                // TODO: parameters
                expect_next_token(lexer::TokenType::LeftParen);
                expect_next_token(lexer::TokenType::RightParen);

                next_token();
                auto body = parse_statement();

                return std::make_shared<ast::FunctionDeclarationStatement>(identifier_token.value, body);
            }

            assert(false);
        }
        case lexer::TokenType::Identifier: {
            return std::make_shared<ast::ExpressionStatement>(parse_expression());
        }
        case lexer::TokenType::LeftBrace: {
            auto s = std::make_shared<ast::BlockStatement>(parse_statements());
            expect_next_token(lexer::TokenType::RightBrace);
            return s;
        }
        default:
            unexpected_token();
            assert(false);
    }
}

std::vector<std::shared_ptr<ast::Statement>> Parser::parse_statements() {
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

ast::Program Parser::parse(std::vector<lexer::Token> input_tokens) {
    tokens = input_tokens;
    ast::Program program;
    program.body = parse_statements();
    return program;
}

}
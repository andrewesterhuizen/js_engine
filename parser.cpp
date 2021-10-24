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
                  << " at " << t.line << ":" << t.column
                  << "\n";
        assert(false);
    }

    return t;
}

void Parser::unexpected_token() {
    auto t = tokens[index];
    std::cerr << "unexpected token \"" << t.value << "\" at " << t.line << ":" << t.column << "\n";
}

bool Parser::next_token_type_is_end_of_expression() {
    return token_type_is_end_of_expression(peek_next_token().type);
}

bool Parser::token_type_is_end_of_expression(lexer::TokenType type) {
    return type == lexer::TokenType::Semicolon ||
           type == lexer::TokenType::RightParen ||
           type == lexer::TokenType::RightBrace ||
           type == lexer::TokenType::RightBracket ||
           type == lexer::TokenType::Colon ||
           type == lexer::TokenType::Comma;
}

bool Parser::token_type_is_assignment_operator(lexer::TokenType type) {
    return type == lexer::TokenType::Equals ||
           type == lexer::TokenType::AdditionAssignment ||
           type == lexer::TokenType::SubtractionAssignment ||
           type == lexer::TokenType::MultiplicationAssignment ||
           type == lexer::TokenType::DivisionAssignment;
}

std::shared_ptr<ast::Expression> Parser::parse_member_expression(std::shared_ptr<ast::Expression> left) {
    auto next = tokens[index];

    if (next.type == lexer::TokenType::Dot) {
        auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
        auto right = std::make_shared<ast::IdentifierExpression>(identifier_token.value);
        return std::make_shared<ast::MemberExpression>(left, right, false);
    }

    if (next.type == lexer::TokenType::LeftBracket) {
        next_token();
        auto right = parse_expression();
        expect_next_token(lexer::TokenType::RightBracket);
        return std::make_shared<ast::MemberExpression>(left, right, true);
    }

    assert(false);
}

std::shared_ptr<ast::Expression> Parser::parse_binary_expression(std::shared_ptr<ast::Expression> left) {
    auto next = tokens[index];

    auto op = ast::token_type_to_operator(next.type);

    next_token();
    auto right = parse_expression();

    return std::make_shared<ast::BinaryExpression>(left, right, op);
}


std::shared_ptr<ast::Expression> Parser::parse_call_expression(std::shared_ptr<ast::Expression> callee) {
    auto next = tokens[index];

    auto call_expression = std::make_shared<ast::CallExpression>(callee);

    assert(next.type == lexer::TokenType::LeftParen);

    next = next_token();
    while (next.type != lexer::TokenType::RightParen) {
        auto arg = parse_expression();
        call_expression->arguments.push_back(arg);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            break;
        }

        next = next_token();
        if (next.type == lexer::TokenType::Comma) {
            next = next_token();
        }
    }

    return call_expression;
}

std::shared_ptr<ast::Expression> Parser::parse_assignment_expression(std::shared_ptr<ast::Expression> left) {
    auto next = tokens[index];

    assert(token_type_is_assignment_operator(next.type));
    auto op = ast::token_type_to_operator(next.type);

    next_token();
    auto right = parse_expression();

    return std::make_shared<ast::AssignmentExpression>(left, right, op);
}

std::shared_ptr<ast::Expression> Parser::parse_expression_recurse(std::shared_ptr<ast::Expression> left) {
    if (peek_next_token().type == lexer::TokenType::Semicolon) {
        return left;
    }

    auto next = next_token();

    // member expression, object.property
    if (next.type == lexer::TokenType::Dot) {
        left = parse_member_expression(left);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        next = next_token();
    }

    // member expression with index, object[property]
    if (next.type == lexer::TokenType::LeftBracket) {
        left = parse_member_expression(left);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        next = next_token();
    }

    // call
    if (next.type == lexer::TokenType::LeftParen) {
        left = parse_call_expression(left);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        next = next_token();
    }

    // assignment
    if (token_type_is_assignment_operator(next.type)) {
        left = parse_assignment_expression(left);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        if (token_type_is_end_of_expression(peek_next_token().type)) {
            return left;
        }

        next = next_token();
    }

    // update
    if (next.type == lexer::TokenType::Increment || next.type == lexer::TokenType::Decrement) {
        left = std::make_shared<ast::UpdateExpression>(left, ast::token_type_to_operator(next.type), false);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        next = next_token();
    }

    // ternary
    if (next.type == lexer::TokenType::QuestionMark) {
        next_token();
        auto consequent = parse_expression();
        expect_next_token(lexer::TokenType::Colon);
        next_token();
        auto alternative = parse_expression();
        left = std::make_shared<ast::TernaryExpression>(left, consequent, alternative);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        next = next_token();
    }

    // binary
    if (ast::token_type_is_operator(next.type)) {
        left = parse_binary_expression(left);

        if (peek_next_token().type == lexer::TokenType::Semicolon) {
            return left;
        }

        if (token_type_is_end_of_expression(peek_next_token().type)) {
            return left;
        }

        next = next_token();
    }

    backup();
    return left;
}

std::shared_ptr<ast::Expression> Parser::parse_expression() {
    auto t = tokens[index];

    switch (t.type) {
        case lexer::TokenType::Number: {
            auto left = std::make_shared<ast::NumberLiteralExpression>(std::stod(t.value));
            return parse_expression_recurse(left);
        }
        case lexer::TokenType::String: {
            auto left = std::make_shared<ast::StringLiteralExpression>(t.value);
            return parse_expression_recurse(left);
        }
        case lexer::TokenType::Identifier: {
            auto left = std::make_shared<ast::IdentifierExpression>(t.value);
            return parse_expression_recurse(left);
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
        case lexer::TokenType::LeftBracket: {
            auto next = next_token();
            auto ae = std::make_shared<ast::ArrayExpression>();

            // get properties
            while (next.type != lexer::TokenType::RightBracket) {
                ae->elements.push_back(parse_expression());

                next = next_token();
                if (next.type == lexer::TokenType::Comma) {
                    next = next_token();
                }
            }

            return ae;
        }
        case lexer::TokenType::Keyword: {
            if (t.value == "var" || t.value == "let" || t.value == "const") {
                auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
                expect_next_token(lexer::TokenType::Equals);

                next_token();
                auto value = parse_expression();

                auto type = ast::get_variable_type(t.value);
                return std::make_shared<ast::VariableDeclarationExpression>(identifier_token.value, value, type);
            }

            if (t.value == "true" || t.value == "false") {
                auto left = std::make_shared<ast::BooleanLiteralExpression>(t.value == "true");

                if (next_token_type_is_end_of_expression()) {
                    return left;
                }

                auto next = next_token();

                auto op = ast::token_type_to_operator(next.type);

                next_token();
                auto right = parse_expression();

                return std::make_shared<ast::BinaryExpression>(left, right, op);
            }

            assert(false);
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
            if (t.value == "var" || t.value == "let" || t.value == "const") {
                auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
                expect_next_token(lexer::TokenType::Equals);

                next_token();
                auto value = parse_expression();

                expect_next_token(lexer::TokenType::Semicolon);

                auto type = ast::get_variable_type(t.value);
                return std::make_shared<ast::VariableDeclarationStatement>(identifier_token.value, value, type);
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
                } else {
                    backup();
                }

                return std::make_shared<ast::IfStatement>(test, consequent, alternative);
            } else if (t.value == "while") {
                expect_next_token(lexer::TokenType::LeftParen);

                next_token();
                auto test = parse_expression();

                expect_next_token(lexer::TokenType::RightParen);

                next_token();
                auto body = parse_statement();

                return std::make_shared<ast::WhileStatement>(test, body);
            } else if (t.value == "for") {
                expect_next_token(lexer::TokenType::LeftParen);

                next_token();

                auto init = parse_expression();
                expect_next_token(lexer::TokenType::Semicolon);

                next_token();
                auto test = parse_expression();
                expect_next_token(lexer::TokenType::Semicolon);

                next_token();
                auto update = parse_expression();

                expect_next_token(lexer::TokenType::RightParen);

                next_token();
                auto body = parse_statement();

                return std::make_shared<ast::ForStatement>(init, test, update, body);
            } else if (t.value == "function") {
                auto identifier_token = expect_next_token(lexer::TokenType::Identifier);

                expect_next_token(lexer::TokenType::LeftParen);

                std::vector<std::string> parameters;

                auto next = next_token();

                while (next.type != lexer::TokenType::RightParen) {
                    assert(next.type == lexer::TokenType::Identifier);
                    parameters.push_back(next.value);

                    next = next_token();
                    if (next.type == lexer::TokenType::Comma) {
                        next = next_token();
                    }
                }

                assert(tokens[index].type == lexer::TokenType::RightParen);

                next_token();
                auto body = parse_statement();

                return std::make_shared<ast::FunctionDeclarationStatement>(identifier_token.value, parameters, body);
            } else if (t.value == "true" || t.value == "false") {
                auto s = std::make_shared<ast::ExpressionStatement>(parse_expression());
                expect_next_token(lexer::TokenType::Semicolon);
                return s;
            } else if (t.value == "return") {
                auto s = std::make_shared<ast::ReturnStatement>();

                auto next = next_token();
                if (next.type != lexer::TokenType::Semicolon) {
                    s->argument = parse_expression();
                    expect_next_token(lexer::TokenType::Semicolon);
                }

                return s;
            }

            assert(false);
        }
        case lexer::TokenType::Number:
        case lexer::TokenType::String:
        case lexer::TokenType::LeftBracket:
        case lexer::TokenType::Identifier: {
            auto s = std::make_shared<ast::ExpressionStatement>(parse_expression());
            expect_next_token(lexer::TokenType::Semicolon);
            return s;
        }
        case lexer::TokenType::LeftParen: {
            next_token();
            auto s = std::make_shared<ast::ExpressionStatement>(parse_expression());
            expect_next_token(lexer::TokenType::RightParen);
            expect_next_token(lexer::TokenType::Semicolon);
            return s;
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

    while (index < tokens.size() && t.type != lexer::TokenType::EndOfFile) {
        switch (t.type) {
            case lexer::TokenType::LeftParen:
            case lexer::TokenType::Keyword:
            case lexer::TokenType::Number:
            case lexer::TokenType::String:
            case lexer::TokenType::LeftBracket:
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
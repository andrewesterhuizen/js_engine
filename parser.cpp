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

std::shared_ptr<ast::Expression> Parser::parse_member_expression(std::shared_ptr<ast::Expression> left) {
    auto next = tokens[index];

    if (next.type == lexer::TokenType::Dot) {
        auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
        auto right = std::make_shared<ast::IdentifierExpression>(identifier_token.value);
        return std::make_shared<ast::MemberExpression>(left, right, false);
    }

    if (next.type == lexer::TokenType::LeftBracket) {
        next_token();
        auto right = parse_expression(nullptr);
        expect_next_token(lexer::TokenType::RightBracket);
        return std::make_shared<ast::MemberExpression>(left, right, true);
    }

    assert(false);
}

std::shared_ptr<ast::Expression> Parser::parse_binary_expression(std::shared_ptr<ast::Expression> left) {
    auto next = tokens[index];

    auto op = ast::token_type_to_operator(next.type);

    next_token();
    auto right = parse_expression(nullptr);

    return std::make_shared<ast::BinaryExpression>(left, right, op);
}


std::shared_ptr<ast::Expression> Parser::parse_call_expression(std::shared_ptr<ast::Expression> callee) {
    auto next = tokens[index];

    auto call_expression = std::make_shared<ast::CallExpression>(callee);

    assert(next.type == lexer::TokenType::LeftParen);

    next = next_token();
    while (next.type != lexer::TokenType::RightParen) {
        auto arg = parse_expression(nullptr);
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

    auto op = ast::token_type_to_operator(next.type);

    next_token();
    auto right = parse_expression(nullptr);

    return std::make_shared<ast::AssignmentExpression>(left, right, op);
}

std::shared_ptr<ast::Expression> Parser::parse_variable_declaration_expression() {
    auto next = tokens[index];
    auto type = ast::get_variable_type(next.value);

    std::vector<std::string> identifiers;
    identifiers.push_back(expect_next_token(lexer::TokenType::Identifier).value);

    if (peek_next_token().type == lexer::TokenType::Semicolon) {
        return std::make_shared<ast::VariableDeclarationExpression>(identifiers, nullptr, type);
    }

    next = next_token();
    while (next.type != lexer::TokenType::Equals) {
        if (next.type != lexer::TokenType::Comma) {
            break;
        }

        identifiers.push_back(expect_next_token(lexer::TokenType::Identifier).value);
        next = next_token();
    }

    assert(next.type == lexer::TokenType::Equals);

    next_token();
    auto value = parse_expression(nullptr);

    return std::make_shared<ast::VariableDeclarationExpression>(identifiers, value, type);
}

std::shared_ptr<ast::Expression> Parser::parse_array_expression() {
    auto expression = std::make_shared<ast::ArrayExpression>();

    auto next = next_token();

    // get properties
    while (next.type != lexer::TokenType::RightBracket) {
        expression->elements.push_back(parse_expression(nullptr));

        next = next_token();
        if (next.type == lexer::TokenType::Comma) {
            next = next_token();
        }
    }

    return expression;
}

std::shared_ptr<ast::Expression> Parser::parse_object_expression() {
    auto expression = std::make_shared<ast::ObjectExpression>();

    auto next = next_token();

    // get properties
    while (next.type != lexer::TokenType::RightBrace) {
        auto id = next;
        assert(id.type == lexer::TokenType::Identifier);

        expect_next_token(lexer::TokenType::Colon);

        next_token();
        auto value = parse_expression(nullptr);
        expression->properties[id.value] = value;

        next = next_token();
        if (next.type == lexer::TokenType::Comma) {
            next = next_token();
        }
    }

    return expression;
}

std::shared_ptr<ast::Expression> Parser::parse_function_expression() {
    std::optional<std::string> identifier;

    if (peek_next_token().type == lexer::TokenType::Identifier) {
        auto identifier_token = expect_next_token(lexer::TokenType::Identifier);
        identifier = identifier_token.value;
    }

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

    return std::make_shared<ast::FunctionExpression>(identifier, parameters, body);
}

std::shared_ptr<ast::Expression> Parser::parse_update_expression(std::shared_ptr<ast::Expression> left) {
    auto t = tokens[index];
    return std::make_shared<ast::UpdateExpression>(left, ast::token_type_to_operator(t.type), false);
}

std::shared_ptr<ast::Expression> Parser::parse_ternary_expression(std::shared_ptr<ast::Expression> left) {
    auto next = tokens[index];

    next_token();
    auto consequent = parse_expression(nullptr);
    expect_next_token(lexer::TokenType::Colon);
    next_token();
    auto alternative = parse_expression(nullptr);

    return std::make_shared<ast::TernaryExpression>(left, consequent, alternative);
}

std::shared_ptr<ast::Expression> Parser::parse_expression(std::shared_ptr<ast::Expression> left) {
    auto t = tokens[index];

    if (left == nullptr) {
        switch (t.type) {
            case lexer::TokenType::LeftParen: {
                next_token();
                auto left = parse_expression(nullptr);
                expect_next_token(lexer::TokenType::RightParen);
                return parse_expression(left);
            }
            case lexer::TokenType::Number: {
                auto left = std::make_shared<ast::NumberLiteralExpression>(std::stod(t.value));
                return parse_expression(left);
            }
            case lexer::TokenType::String: {
                auto left = std::make_shared<ast::StringLiteralExpression>(t.value);
                return parse_expression(left);
            }
            case lexer::TokenType::Identifier: {
                auto left = std::make_shared<ast::IdentifierExpression>(t.value);
                return parse_expression(left);
            }
            case lexer::TokenType::LeftBrace: {
                return parse_expression(parse_object_expression());
            }
            case lexer::TokenType::LeftBracket: {
                return parse_expression(parse_array_expression());
            }
            case lexer::TokenType::Keyword: {
                if (t.value == "var" || t.value == "let" || t.value == "const") {
                    return parse_expression(parse_variable_declaration_expression());
                }

                if (t.value == "true" || t.value == "false") {
                    return parse_expression(std::make_shared<ast::BooleanLiteralExpression>(t.value == "true"));
                }

                if (t.value == "function") {
                    return parse_expression(parse_function_expression());
                }

                assert(false);
            }
            default:
                unexpected_token();
                assert(false);
        }
    }

    auto next = next_token();

    switch (next.type) {
        case lexer::TokenType::Semicolon:
        case lexer::TokenType::RightParen:
        case lexer::TokenType::RightBrace:
        case lexer::TokenType::RightBracket:
        case lexer::TokenType::Colon:
        case lexer::TokenType::Comma: {
            // end of expression
            backup();
            return left;
        }
        case lexer::TokenType::Dot:
        case lexer::TokenType::LeftBracket:
            return parse_expression(parse_member_expression(left));
        case lexer::TokenType::LeftParen:
            return parse_expression(parse_call_expression(left));
        case lexer::TokenType::Increment:
        case lexer::TokenType::Decrement:
            return parse_expression(parse_update_expression(left));
        case lexer::TokenType::QuestionMark:
            return parse_expression(parse_ternary_expression(left));
        case lexer::TokenType::Equals:
        case lexer::TokenType::AdditionAssignment:
        case lexer::TokenType::SubtractionAssignment:
        case lexer::TokenType::MultiplicationAssignment:
        case lexer::TokenType::DivisionAssignment:
            return parse_expression(parse_assignment_expression(left));
        default:
            assert(ast::token_type_is_operator(next.type));
            return parse_expression(parse_binary_expression(left));
    }
}

std::shared_ptr<ast::Statement> Parser::parse_statement() {
    auto t = tokens[index];

    switch (t.type) {
        case lexer::TokenType::Keyword: {
            if (t.value == "var" || t.value == "let" || t.value == "const") {
                auto s = std::make_shared<ast::ExpressionStatement>(parse_expression(nullptr));
                expect_next_token(lexer::TokenType::Semicolon);
                return s;
            } else if (t.value == "if") {
                std::shared_ptr<ast::Statement> alternative = nullptr;

                expect_next_token(lexer::TokenType::LeftParen);

                next_token();
                auto test = parse_expression(nullptr);

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
                auto test = parse_expression(nullptr);

                expect_next_token(lexer::TokenType::RightParen);

                next_token();
                auto body = parse_statement();

                return std::make_shared<ast::WhileStatement>(test, body);
            } else if (t.value == "for") {
                expect_next_token(lexer::TokenType::LeftParen);

                next_token();

                auto init = parse_expression(nullptr);
                expect_next_token(lexer::TokenType::Semicolon);

                next_token();
                auto test = parse_expression(nullptr);
                expect_next_token(lexer::TokenType::Semicolon);

                next_token();
                auto update = parse_expression(nullptr);

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
                auto s = std::make_shared<ast::ExpressionStatement>(parse_expression(nullptr));
                expect_next_token(lexer::TokenType::Semicolon);
                return s;
            } else if (t.value == "return") {
                auto s = std::make_shared<ast::ReturnStatement>();

                auto next = next_token();
                if (next.type != lexer::TokenType::Semicolon) {
                    s->argument = parse_expression(nullptr);
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
            auto s = std::make_shared<ast::ExpressionStatement>(parse_expression(nullptr));
            expect_next_token(lexer::TokenType::Semicolon);
            return s;
        }
        case lexer::TokenType::LeftParen: {
            auto s = std::make_shared<ast::ExpressionStatement>(parse_expression(nullptr));
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
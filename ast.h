#pragma once

#include <string>
#include <vector>

#include "lexer.h"
#include "json.hpp"

namespace ast {

enum class Operator {
    Plus,
    Equals
};

Operator token_type_to_operator(lexer::TokenType token_type);
std::string operator_to_string(Operator op);

enum class ExpressionType {
    Call,
    Member,
    Identifier,
    NumberLiteral,
    StringLiteral,
    BooleanLiteral,
    Binary,
    Assignment
};

enum class StatementType {
    Expression,
    Block,
    If,
    FunctionDeclaration,
    VariableDeclaration
};

struct Statement {
    Statement(StatementType type) : type(type) {}
    StatementType type;
    virtual nlohmann::json to_json() = 0;
};

struct Expression {
    Expression(ExpressionType type) : type(type) {}
    ExpressionType type;
    virtual nlohmann::json to_json() = 0;
};

struct ExpressionStatement : public Statement {
    ExpressionStatement(std::shared_ptr<Expression> expression)
            : Statement(StatementType::Expression), expression(expression) {}
    std::shared_ptr<Expression> expression;
    nlohmann::json to_json() override;
};

struct BlockStatement : public Statement {
    BlockStatement(std::vector<std::shared_ptr<Statement>> body)
            : Statement(StatementType::Block), body(body) {}
    std::vector<std::shared_ptr<Statement>> body;
    nlohmann::json to_json() override;
};

struct IfStatement : public Statement {
    IfStatement(std::shared_ptr<Expression> test, std::shared_ptr<Statement> consequent,
                std::shared_ptr<Statement> alternative)
            : Statement(StatementType::If), test(test), consequent(consequent), alternative(alternative) {}
    std::shared_ptr<Expression> test;
    std::shared_ptr<Statement> consequent;
    std::shared_ptr<Statement> alternative;
    nlohmann::json to_json() override;
};

struct FunctionDeclarationStatement : public Statement {
    FunctionDeclarationStatement(std::string identifier, std::shared_ptr<Statement> body)
            : Statement(StatementType::FunctionDeclaration), identifier(identifier), body(body) {}
    std::string identifier;
    std::shared_ptr<Statement> body;
    nlohmann::json to_json() override;
};

struct VariableDeclarationStatement : public Statement {
    VariableDeclarationStatement(std::string identifier, std::shared_ptr<Expression> value)
            : Statement(StatementType::VariableDeclaration), identifier(identifier), value(value) {}
    std::string identifier;
    std::shared_ptr<Expression> value;
    nlohmann::json to_json() override;
};

struct CallExpression : public Expression {
    CallExpression(std::shared_ptr<Expression> callee)
            : Expression(ExpressionType::Call), callee(callee) {}
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;
    nlohmann::json to_json() override;
};

struct MemberExpression : public Expression {
    MemberExpression(std::shared_ptr<Expression> object, std::shared_ptr<Expression> property) :
            Expression(ExpressionType::Member), object(object), property(property) {}
    std::shared_ptr<Expression> object;
    std::shared_ptr<Expression> property;
    nlohmann::json to_json() override;
};

struct IdentifierExpression : public Expression {
    IdentifierExpression(std::string name) : Expression(ExpressionType::Identifier), name(name) {}
    std::string name;
    nlohmann::json to_json() override;
};

struct NumberLiteralExpression : public Expression {
    NumberLiteralExpression(double value) : Expression(ExpressionType::NumberLiteral), value(value) {}
    double value;
    nlohmann::json to_json() override;
};

struct StringLiteralExpression : public Expression {
    StringLiteralExpression(std::string value) : Expression(ExpressionType::StringLiteral), value(value) {}
    std::string value;
    nlohmann::json to_json() override;
};

struct BooleanLiteralExpression : public Expression {
    BooleanLiteralExpression(bool value) : Expression(ExpressionType::BooleanLiteral), value(value) {}
    bool value;
    nlohmann::json to_json() override;
};

struct BinaryExpression : public Expression {
    BinaryExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right, Operator op)
            : Expression(ExpressionType::Binary), left(left), right(right), op(op) {}
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    Operator op;
    nlohmann::json to_json() override;
};

struct AssignmentExpression : public Expression {
    AssignmentExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right, Operator op)
            : Expression(ExpressionType::Assignment), left(left), right(right), op(op) {}
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    Operator op;
    nlohmann::json to_json() override;
};

struct Program {
    std::vector<std::shared_ptr<Statement>> body;
    nlohmann::json to_json();
};

}
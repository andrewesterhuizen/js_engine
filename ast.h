#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "lexer.h"
#include "json.hpp"

namespace ast {

enum class Operator {
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,
    Increment,
    Decrement,
    Equals,
    EqualTo,
    EqualToStrict,
    And,
    Or,
    NotEqualTo,
    GreaterThan,
    GreaterThanOrEqualTo,
    LessThan,
    LessThanOrEqualTo
};

Operator token_type_to_operator(lexer::TokenType token_type);
bool token_type_is_operator(lexer::TokenType token_type);
std::string operator_to_string(Operator op);

enum class ExpressionType {
    VariableDeclaration,
    Call,
    Member,
    Identifier,
    NumberLiteral,
    StringLiteral,
    BooleanLiteral,
    Binary,
    Assignment,
    Object,
    Array,
    Update
};

enum class StatementType {
    Expression,
    Block,
    If,
    FunctionDeclaration,
    VariableDeclaration, // TODO: this should be removed and replace with expression version
    While,
    For
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

struct ForStatement : public Statement {
    ForStatement(std::shared_ptr<Expression> init, std::shared_ptr<Expression> test, std::shared_ptr<Expression> update,
                 std::shared_ptr<Statement> body)
            : Statement(StatementType::For), init(init), test(test), update(update), body(body) {}
    std::shared_ptr<Expression> init;
    std::shared_ptr<Expression> test;
    std::shared_ptr<Expression> update;
    std::shared_ptr<Statement> body;
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

struct WhileStatement : public Statement {
    WhileStatement(std::shared_ptr<Expression> test, std::shared_ptr<Statement> body)
            : Statement(StatementType::While), test(test), body(body) {}
    std::shared_ptr<Expression> test;
    std::shared_ptr<Statement> body;
    nlohmann::json to_json() override;
};

struct CallExpression : public Expression {
    CallExpression(std::shared_ptr<Expression> callee)
            : Expression(ExpressionType::Call), callee(callee) {}
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;
    nlohmann::json to_json() override;
};

struct VariableDeclarationExpression : public Expression {
    VariableDeclarationExpression(std::string identifier, std::shared_ptr<Expression> value)
            : Expression(ExpressionType::VariableDeclaration), identifier(identifier), value(value) {}
    std::string identifier;
    std::shared_ptr<Expression> value;
    nlohmann::json to_json() override;
};

struct MemberExpression : public Expression {
    MemberExpression(std::shared_ptr<Expression> object, std::shared_ptr<Expression> property, bool is_computed) :
            Expression(ExpressionType::Member), object(object), property(property), is_computed(is_computed) {}
    std::shared_ptr<Expression> object;
    std::shared_ptr<Expression> property;
    bool is_computed;
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

struct UpdateExpression : public Expression {
    UpdateExpression(std::shared_ptr<Expression> argument, Operator op, bool is_prefix)
            : Expression(ExpressionType::Update), argument(argument), op(op), is_prefix(is_prefix) {}
    std::shared_ptr<Expression> argument;
    Operator op;
    bool is_prefix;
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

struct ObjectExpression : public Expression {
    ObjectExpression() : Expression(ExpressionType::Object) {}
    std::unordered_map<std::string, std::shared_ptr<Expression>> properties;
    nlohmann::json to_json() override;
};

struct ArrayExpression : public Expression {
    ArrayExpression() : Expression(ExpressionType::Array) {}
    std::vector<std::shared_ptr<Expression>> elements;
    nlohmann::json to_json() override;
};

struct Program {
    std::vector<std::shared_ptr<Statement>> body;
    nlohmann::json to_json();
};

}
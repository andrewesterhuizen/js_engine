#pragma once

#include <string>
#include <vector>

#include "lexer.h"
#include "json.hpp"

namespace ast {

enum class Operator {
    Plus
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
    Binary
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
    std::shared_ptr<Expression> expression;
    ExpressionStatement() : Statement(StatementType::Expression) {}

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "ExpressionStatement";
        j["expression"] = expression->to_json();
        return j;
    }
};

struct BlockStatement : public Statement {
    std::vector<std::shared_ptr<Statement>> body;
    BlockStatement() : Statement(StatementType::Block) {}

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "BlockStatement";

        std::vector<nlohmann::json> statements;
        for (auto s: body) {
            statements.push_back(s->to_json());
        }
        j["body"] = statements;

        return j;
    }
};

struct IfStatement : public Statement {
    std::shared_ptr<Expression> test;
    std::shared_ptr<Statement> consequent;
    std::shared_ptr<Statement> alternative;
    IfStatement() : Statement(StatementType::If) {}

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "IfStatement";
        j["test"] = test->to_json();
        j["consequent"] = consequent->to_json();
        j["alternative"] = alternative->to_json();
        return j;
    }
};

struct FunctionDeclarationStatement : public Statement {
    std::string identifier;
    std::shared_ptr<Statement> body;
    FunctionDeclarationStatement() : Statement(StatementType::FunctionDeclaration) {}

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "FunctionDeclarationStatement";
        j["identifier"] = identifier;
        j["body"] = body->to_json();
        return j;
    }
};

struct VariableDeclarationStatement : public Statement {
    std::string identifier;
    std::shared_ptr<Expression> value;

    VariableDeclarationStatement() : Statement(StatementType::VariableDeclaration) {}

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "VariableDeclarationStatement";
        j["identifier"] = identifier;
        j["value"] = value->to_json();
        return j;
    }
};

struct CallExpression : public Expression {
    CallExpression() : Expression(ExpressionType::Call) {}
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "CallExpression";
        j["callee"] = callee->to_json();

        std::vector<nlohmann::json> args;

        for (auto arg: arguments) {
            args.push_back(arg->to_json());
        }

        j["expression"] = args;
        return j;
    }
};

struct MemberExpression : public Expression {
    MemberExpression() : Expression(ExpressionType::Member) {}
    std::shared_ptr<Expression> object;
    std::shared_ptr<Expression> property;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "MemberExpression";
        j["object"] = object->to_json();
        j["property"] = property->to_json();
        return j;
    }
};

struct IdentifierExpression : public Expression {
    IdentifierExpression(std::string name) : Expression(ExpressionType::Identifier), name(name) {}
    std::string name;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "IdentifierExpression";
        j["name"] = name;
        return j;
    }
};

struct NumberLiteralExpression : public Expression {
    NumberLiteralExpression(double value) : Expression(ExpressionType::NumberLiteral), value(value) {}
    double value;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "NumberLiteralExpression";
        j["value"] = value;
        return j;
    }
};

struct StringLiteralExpression : public Expression {
    StringLiteralExpression(std::string value) : Expression(ExpressionType::StringLiteral), value(value) {}
    std::string value;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "StringLiteralExpression";
        j["value"] = value;
        return j;
    }
};

struct BooleanLiteralExpression : public Expression {
    BooleanLiteralExpression(bool value) : Expression(ExpressionType::BooleanLiteral), value(value) {}
    bool value;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "BooleanLiteralExpression";
        j["value"] = value;
        return j;
    }
};

struct BinaryExpression : public Expression {
    BinaryExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right, Operator op)
            : Expression(ExpressionType::Binary), left(left), right(right), op(op) {}
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    Operator op;

    nlohmann::json to_json() override {
        nlohmann::json j;
        j["type"] = "BinaryExpression";
        j["left"] = left->to_json();
        j["left"] = right->to_json();
        j["op"] = operator_to_string(op);
        return j;
    }
};

struct Program {
    std::vector<std::shared_ptr<Statement>> body;

    nlohmann::json to_json() {

        nlohmann::json j;
        j["type"] = "Program";

        std::vector<nlohmann::json> statements;
        for (auto s: body) {
            statements.push_back(s->to_json());
        }
        j["body"] = statements;

        return j;
    }
};

}
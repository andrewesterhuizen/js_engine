#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "lexer.h"
#include "json.hpp"

namespace ast {

#define OPERATORS(MAP) \
    MAP(Plus) \
    MAP(Minus) \
    MAP(Multiply) \
    MAP(Divide) \
    MAP(Modulo) \
    MAP(Increment) \
    MAP(Decrement) \
    MAP(AdditionAssignment) \
    MAP(SubtractionAssignment) \
    MAP(MultiplicationAssignment) \
    MAP(DivisionAssignment) \
    MAP(Exponentiation) \
    MAP(Equals) \
    MAP(EqualTo) \
    MAP(EqualToStrict) \
    MAP(And) \
    MAP(Or) \
    MAP(NotEqualTo) \
    MAP(NotEqualToStrict) \
    MAP(GreaterThan) \
    MAP(GreaterThanOrEqualTo) \
    MAP(LessThan) \
    MAP(LessThanOrEqualTo)  \
    MAP(Not)

#define CREATE_ENUM(NAME) NAME,

enum class Operator {
    OPERATORS(CREATE_ENUM)
};

Operator token_type_to_operator(lexer::TokenType token_type);
bool token_type_is_operator(lexer::TokenType token_type);
std::string operator_to_string(Operator op);

enum class VariableType {
    Var,
    Let,
    Const
};

VariableType get_variable_type(std::string type);

#define EXPRESSIONS(MAP) \
    MAP(VariableDeclaration) \
    MAP(Call) \
    MAP(Member) \
    MAP(Identifier) \
    MAP(NumberLiteral) \
    MAP(StringLiteral) \
    MAP(BooleanLiteral) \
    MAP(Binary) \
    MAP(Assignment) \
    MAP(Object) \
    MAP(Array) \
    MAP(Update) \
    MAP(Ternary) \
    MAP(Function) \
    MAP(ArrowFunction) \
    MAP(Unary) \
    MAP(This) \
    MAP(New)

#define STATEMENTS(MAP) \
    MAP(Expression) \
    MAP(Block) \
    MAP(If) \
    MAP(FunctionDeclaration) \
    MAP(While) \
    MAP(For) \
    MAP(Return)


enum class ExpressionType {
    EXPRESSIONS(CREATE_ENUM)
};

enum class StatementType {
    STATEMENTS(CREATE_ENUM)
};

#undef CREATE_ENUM

#define FORWARD_DECLARE_STATEMENT(NAME) struct NAME ## Statement;
STATEMENTS(FORWARD_DECLARE_STATEMENT)
#undef FORWARD_DECLARE_STATEMENT

#define FORWARD_DECLARE_EXPRESSION(NAME) struct NAME ## Expression;
EXPRESSIONS(FORWARD_DECLARE_EXPRESSION)
#undef FORWARD_DECLARE_EXPRESSION

struct Statement {
    Statement(StatementType type) : type(type) {}
    StatementType type;
    virtual nlohmann::json to_json() = 0;

    template<typename T>
    T* as();
    ExpressionStatement* as_expression_statement();
    BlockStatement* as_block();
    IfStatement* as_if();
    FunctionDeclarationStatement* as_function_declaration();
    WhileStatement* as_while();
    ForStatement* as_for();
    ReturnStatement* as_return();
};

struct Expression {
    Expression(ExpressionType type) : type(type) {}
    ExpressionType type;
    virtual nlohmann::json to_json() = 0;

    template<typename T>
    T* as();
    NumberLiteralExpression* as_number_literal();
    StringLiteralExpression* as_string_literal();
    BooleanLiteralExpression* as_boolean_literal();
    ArrayExpression* as_array();
    ObjectExpression* as_object();
    FunctionExpression* as_function();
    ArrowFunctionExpression* as_arrow_function();
    IdentifierExpression* as_identifier();
    CallExpression* as_call();
    VariableDeclarationExpression* as_variable_declaration();
    MemberExpression* as_member();
    BinaryExpression* as_binary();
    UnaryExpression* as_unary();
    AssignmentExpression* as_assignment();
    UpdateExpression* as_update();
    TernaryExpression* as_ternary();
    NewExpression* as_new();
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

struct ReturnStatement : public Statement {
    ReturnStatement() : Statement(StatementType::Return), argument(nullptr) {}
    ReturnStatement(std::shared_ptr<Expression> argument) : Statement(StatementType::Return), argument(argument) {}
    std::shared_ptr<Expression> argument;
    nlohmann::json to_json() override;
};

struct FunctionDeclarationStatement : public Statement {
    FunctionDeclarationStatement(std::string identifier, std::vector<std::string> parameters,
                                 std::shared_ptr<Statement> body)
            : Statement(StatementType::FunctionDeclaration), identifier(identifier), parameters(parameters),
              body(body) {}
    std::string identifier;
    std::vector<std::string> parameters;
    std::shared_ptr<Statement> body;
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
    VariableDeclarationExpression(std::vector<std::string> identifiers, std::shared_ptr<Expression> value,
                                  VariableType type)
            : Expression(ExpressionType::VariableDeclaration), identifiers(identifiers), value(value), type(type) {}
    std::vector<std::string> identifiers;
    std::shared_ptr<Expression> value;
    VariableType type;
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

struct UnaryExpression : public Expression {
    UnaryExpression(std::shared_ptr<Expression> argument, Operator op)
            : Expression(ExpressionType::Unary), argument(argument), op(op) {}
    std::shared_ptr<Expression> argument;
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

struct TernaryExpression : public Expression {
    TernaryExpression(std::shared_ptr<Expression> test, std::shared_ptr<Expression> consequent,
                      std::shared_ptr<Expression> alternative)
            : Expression(ExpressionType::Ternary), test(test), consequent(consequent), alternative(alternative) {}
    std::shared_ptr<Expression> test;
    std::shared_ptr<Expression> consequent;
    std::shared_ptr<Expression> alternative;
    nlohmann::json to_json() override;
};

struct FunctionExpression : public Expression {
    FunctionExpression(std::optional<std::string> identifier, std::vector<std::string> parameters,
                       std::shared_ptr<Statement> body)
            : Expression(ExpressionType::Function), identifier(identifier), parameters(parameters),
              body(body) {}
    std::optional<std::string> identifier;
    std::vector<std::string> parameters;
    std::shared_ptr<Statement> body;
    nlohmann::json to_json() override;
};

struct ArrowFunctionExpression : public Expression {
    ArrowFunctionExpression(std::vector<std::string> parameters, std::shared_ptr<Statement> body)
            : Expression(ExpressionType::ArrowFunction), parameters(parameters),
              body(body) {}
    std::vector<std::string> parameters;
    std::shared_ptr<Statement> body;
    nlohmann::json to_json() override;
};

struct ThisExpression : public Expression {
    ThisExpression() : Expression(ExpressionType::This) {}
    nlohmann::json to_json() override;
};

struct NewExpression : public Expression {
    NewExpression(std::shared_ptr<Expression> callee) : Expression(ExpressionType::New), callee(callee) {}
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;
    nlohmann::json to_json() override;
};

struct Program {
    std::vector<std::shared_ptr<Statement>> body;
    nlohmann::json to_json();
};

}
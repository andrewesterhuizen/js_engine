#include "ast.h"

#include <iostream>

namespace ast {

std::string operator_to_string(Operator op) {
    switch (op) {
        case Operator::Plus:
            return "+";
        case Operator::Minus:
            return "-";
        case Operator::Multiply:
            return "*";
        case Operator::Divide:
            return "/";
        case Operator::Modulo:
            return "%";
        case Operator::EqualTo:
            return "==";
        case Operator::EqualToStrict:
            return "===";
        case Operator::And:
            return "&&";
        case Operator::Or:
            return "||";
        case Operator::NotEqualTo:
            return "!=";
        case Operator::GreaterThan:
            return ">";
        case Operator::GreaterThanOrEqualTo:
            return ">=";
        case Operator::LessThan:
            return "<";
        case Operator::LessThanOrEqualTo:
            return "<=";
        case Operator::Equals:
            return "=";
        case Operator::Increment:
            return "++";
        case Operator::Decrement:
            return "--";
        case Operator::AdditionAssignment:
            return "+=";
        case Operator::SubtractionAssignment:
            return "-=";
        case Operator::MultiplicationAssignment:
            return "*=";
        case Operator::DivisionAssignment:
            return "/=";
    }

    std::cerr << "missing case for Operator in operator_to_string";
    assert(false);
}

Operator token_type_to_operator(lexer::TokenType token_type) {
    switch (token_type) {
        case lexer::TokenType::Plus:
            return Operator::Plus;
        case lexer::TokenType::Minus:
            return Operator::Minus;
        case lexer::TokenType::Asterisk:
            return Operator::Multiply;
        case lexer::TokenType::Slash:
            return Operator::Divide;
        case lexer::TokenType::Percent:
            return Operator::Modulo;
        case lexer::TokenType::EqualTo:
            return Operator::EqualTo;
        case lexer::TokenType::AdditionAssignment:
            return Operator::AdditionAssignment;
        case lexer::TokenType::SubtractionAssignment:
            return Operator::SubtractionAssignment;
        case lexer::TokenType::MultiplicationAssignment:
            return Operator::MultiplicationAssignment;
        case lexer::TokenType::DivisionAssignment:
            return Operator::DivisionAssignment;
        case lexer::TokenType::EqualToStrict:
            return Operator::EqualToStrict;
        case lexer::TokenType::And:
            return Operator::And;
        case lexer::TokenType::Or:
            return Operator::Or;
        case lexer::TokenType::NotEqualTo:
            return Operator::NotEqualTo;
        case lexer::TokenType::GreaterThan:
            return Operator::GreaterThan;
        case lexer::TokenType::GreaterThanOrEqualTo:
            return Operator::GreaterThanOrEqualTo;
        case lexer::TokenType::LessThan:
            return Operator::LessThan;
        case lexer::TokenType::LessThanOrEqualTo:
            return Operator::LessThanOrEqualTo;
        case lexer::TokenType::Equals:
            return Operator::Equals;
        case lexer::TokenType::Increment:
            return Operator::Increment;
        case lexer::TokenType::Decrement:
            return Operator::Decrement;
        default:
            std::cerr << "missing case for TokenType in token_type_to_operator";
            assert(false);
    }
}

bool token_type_is_operator(lexer::TokenType token_type) {
    switch (token_type) {
        case lexer::TokenType::Plus:
        case lexer::TokenType::Minus:
        case lexer::TokenType::Asterisk:
        case lexer::TokenType::Slash:
        case lexer::TokenType::Percent:
        case lexer::TokenType::EqualTo:
        case lexer::TokenType::EqualToStrict:
        case lexer::TokenType::And:
        case lexer::TokenType::Or:
        case lexer::TokenType::NotEqualTo:
        case lexer::TokenType::GreaterThan:
        case lexer::TokenType::GreaterThanOrEqualTo:
        case lexer::TokenType::LessThan:
        case lexer::TokenType::LessThanOrEqualTo:
        case lexer::TokenType::Equals:
        case lexer::TokenType::Increment:
        case lexer::TokenType::Decrement:
            return true;
        default:
            return false;
    }
}

nlohmann::json ExpressionStatement::to_json() {
    nlohmann::json j;
    j["type"] = "ExpressionStatement";
    j["expression"] = expression->to_json();
    return j;
}

nlohmann::json BlockStatement::to_json() {
    nlohmann::json j;
    j["type"] = "BlockStatement";

    std::vector<nlohmann::json> statements;
    for (auto s: body) {
        statements.push_back(s->to_json());
    }
    j["body"] = statements;

    return j;
}

nlohmann::json IfStatement::to_json() {
    nlohmann::json j;
    j["type"] = "IfStatement";
    j["test"] = test->to_json();
    j["consequent"] = consequent->to_json();
    j["alternative"] = alternative->to_json();
    return j;
}

nlohmann::json ForStatement::to_json() {
    nlohmann::json j;
    j["type"] = "ForStatement";
    j["init"] = init->to_json();
    j["test"] = test->to_json();
    j["update"] = update->to_json();
    j["body"] = body->to_json();
    return j;
}

nlohmann::json FunctionDeclarationStatement::to_json() {
    nlohmann::json j;
    j["type"] = "FunctionDeclarationStatement";
    j["identifier"] = identifier;
    j["body"] = body->to_json();
    return j;
}

nlohmann::json VariableDeclarationStatement::to_json() {
    nlohmann::json j;
    j["type"] = "VariableDeclarationStatement";
    j["identifier"] = identifier;
    j["value"] = value->to_json();
    return j;
}

nlohmann::json WhileStatement::to_json() {
    nlohmann::json j;
    j["type"] = "WhileStatement";
    j["test"] = test->to_json();
    j["body"] = body->to_json();
    return j;
}

nlohmann::json CallExpression::to_json() {
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

nlohmann::json MemberExpression::to_json() {
    nlohmann::json j;
    j["type"] = "MemberExpression";
    j["object"] = object->to_json();
    j["property"] = property->to_json();
    j["is_computed"] = is_computed;
    return j;
}

nlohmann::json IdentifierExpression::to_json() {
    nlohmann::json j;
    j["type"] = "IdentifierExpression";
    j["name"] = name;
    return j;
}

nlohmann::json NumberLiteralExpression::to_json() {
    nlohmann::json j;
    j["type"] = "NumberLiteralExpression";
    j["value"] = value;
    return j;
}

nlohmann::json StringLiteralExpression::to_json() {
    nlohmann::json j;
    j["type"] = "StringLiteralExpression";
    j["value"] = value;
    return j;
}

nlohmann::json BooleanLiteralExpression::to_json() {
    nlohmann::json j;
    j["type"] = "BooleanLiteralExpression";
    j["value"] = value;
    return j;
}

nlohmann::json BinaryExpression::to_json() {
    nlohmann::json j;
    j["type"] = "BinaryExpression";
    j["left"] = left->to_json();
    j["left"] = right->to_json();
    j["op"] = operator_to_string(op);
    return j;
}

nlohmann::json UpdateExpression::to_json() {
    nlohmann::json j;
    j["type"] = "UpdateExpression";
    j["argument"] = argument->to_json();
    j["op"] = operator_to_string(op);
    j["is_prefix"] = is_prefix;
    return j;
}

nlohmann::json AssignmentExpression::to_json() {
    nlohmann::json j;
    j["type"] = "AssignmentExpression";
    j["left"] = left->to_json();
    j["right"] = right->to_json();
    j["op"] = operator_to_string(op);
    return j;
}

nlohmann::json VariableDeclarationExpression::to_json() {
    nlohmann::json j;
    j["type"] = "VariableDeclarationExpression";
    j["identifier"] = identifier;
    j["value"] = value->to_json();
    return j;
}

nlohmann::json ObjectExpression::to_json() {
    nlohmann::json j;
    j["type"] = "ObjectExpression";

    j["properties"] = nlohmann::json();

    for (auto p: properties) {
        j["properties"][p.first] = p.second->to_json();
    }

    return j;
}

nlohmann::json ArrayExpression::to_json() {
    nlohmann::json j;
    j["type"] = "ArrayExpression";

    std::vector<nlohmann::json> elements_json;

    for (auto p: elements) {
        elements_json.push_back(p->to_json());
    }

    j["elements"] = elements_json;

    return j;
}

nlohmann::json Program::to_json() {

    nlohmann::json j;
    j["type"] = "Program";

    std::vector<nlohmann::json> statements;
    for (auto s: body) {
        statements.push_back(s->to_json());
    }
    j["body"] = statements;

    return j;
}

}
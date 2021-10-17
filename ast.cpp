#include "ast.h"

#include <iostream>

namespace ast {

std::string operator_to_string(Operator op) {
    switch (op) {
        case Operator::Plus:
            return "+";
        case Operator::Equals:
            return "=";
    }

    std::cerr << "missing case for Operator in operator_to_string";
    assert(false);
}

Operator token_type_to_operator(lexer::TokenType token_type) {
    switch (token_type) {
        case lexer::TokenType::Plus:
            return Operator::Plus;
        case lexer::TokenType::Equals:
            return Operator::Equals;
        default:
            std::cerr << "missing case for TokenType in token_type_to_operator";
            assert(false);
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

nlohmann::json AssignmentExpression::to_json() {
    nlohmann::json j;
    j["type"] = "AssignmentExpression";
    j["left"] = left->to_json();
    j["left"] = right->to_json();
    j["op"] = operator_to_string(op);
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
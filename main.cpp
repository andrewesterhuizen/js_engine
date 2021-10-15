#include <iostream>

#include "ast.h"
#include "interpreter.h"

int main() {
    // variable declaration
    auto var_declaration = std::make_shared<ast::VariableDeclarationStatement>();
    var_declaration->identifier = std::make_shared<ast::IdentifierExpression>("my_string");
    var_declaration->value = std::make_shared<ast::StringLiteralExpression>("hello from variable");

    // console log variable
    auto console_log_member_expression = std::make_shared<ast::MemberExpression>();
    console_log_member_expression->object = std::make_shared<ast::IdentifierExpression>("console");
    console_log_member_expression->expression = std::make_shared<ast::IdentifierExpression>("log");

    auto console_log_call_expression = std::make_shared<ast::CallExpression>();
    console_log_call_expression->callee = console_log_member_expression;
    console_log_call_expression->arguments = std::vector<std::shared_ptr<ast::Expression>>();
    console_log_call_expression->arguments.push_back(std::make_shared<ast::IdentifierExpression>("my_string"));

    auto console_log_call_expression_statement = std::make_shared<ast::ExpressionStatement>();
    console_log_call_expression_statement->expression = console_log_call_expression;

    // program
    ast::Program program;
    program.body.push_back(var_declaration);
    program.body.push_back(console_log_call_expression_statement);

    std::cout << program.to_json().dump(4) << "\n";

    // run ast
    interpreter::Interpreter i;
    i.run(program);

    return 0;
}

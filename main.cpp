#include <iostream>

#include "ast.h"
#include "interpreter.h"

int main() {
    // function declaration
    auto console_log_member_expression = std::make_shared<ast::MemberExpression>();
    console_log_member_expression->object = std::make_shared<ast::IdentifierExpression>("console");
    console_log_member_expression->expression = std::make_shared<ast::IdentifierExpression>("log");

    auto console_log_call_expression = std::make_shared<ast::CallExpression>();
    console_log_call_expression->callee = console_log_member_expression;
    console_log_call_expression->arguments = std::vector<std::shared_ptr<ast::Expression>>();
    console_log_call_expression->arguments.push_back(
            std::make_shared<ast::StringLiteralExpression>("hello from function"));

    auto console_log_call_expression_statement = std::make_shared<ast::ExpressionStatement>();
    console_log_call_expression_statement->expression = console_log_call_expression;

    auto function_body = std::make_shared<ast::BlockStatement>();
    function_body->body.push_back(console_log_call_expression_statement);

    auto function_declaration = std::make_shared<ast::FunctionDeclarationStatement>();
    function_declaration->identifier = std::make_shared<ast::IdentifierExpression>("test");
    function_declaration->body = function_body;


    // function call
    auto test_func_call_expression = std::make_shared<ast::CallExpression>();
    test_func_call_expression->callee = console_log_member_expression;
    test_func_call_expression->arguments = std::vector<std::shared_ptr<ast::Expression>>();
    test_func_call_expression->arguments.push_back(
            std::make_shared<ast::StringLiteralExpression>("hello from function"));

    auto test_func_call_expression_statement = std::make_shared<ast::ExpressionStatement>();
    test_func_call_expression_statement->expression = test_func_call_expression;

    // program
    ast::Program program;
    program.body.push_back(function_declaration);
    program.body.push_back(test_func_call_expression_statement);

    std::cout << program.to_json().dump(4) << "\n";

    // run ast
    interpreter::Interpreter i;
    i.run(program);

    return 0;
}

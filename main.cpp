#include <iostream>

#include "ast.h"
#include "interpreter.h"

int main() {
    // string args
    auto its_true_string = std::make_shared<ast::StringLiteralExpression>("it's true");
    auto its_false_string = std::make_shared<ast::StringLiteralExpression>("it's false");

    // consequent block
    auto if_console_log_member_expr = std::make_shared<ast::MemberExpression>();
    if_console_log_member_expr->object = std::make_shared<ast::IdentifierExpression>("console");
    if_console_log_member_expr->expression = std::make_shared<ast::IdentifierExpression>("log");

    auto if_console_log_call_expr = std::make_shared<ast::CallExpression>();
    if_console_log_call_expr->callee = if_console_log_member_expr;
    if_console_log_call_expr->arguments = std::vector<std::shared_ptr<ast::Expression>>();
    if_console_log_call_expr->arguments.push_back(its_true_string);

    auto if_expr_statement = std::make_shared<ast::ExpressionStatement>();
    if_expr_statement->expression = if_console_log_call_expr;

    auto if_block_statement = std::make_shared<ast::BlockStatement>();
    if_block_statement->body.push_back(if_expr_statement);

    // alternative block
    auto else_console_log_member_expr = std::make_shared<ast::MemberExpression>();
    else_console_log_member_expr->object = std::make_shared<ast::IdentifierExpression>("console");
    else_console_log_member_expr->expression = std::make_shared<ast::IdentifierExpression>("log");

    auto else_console_log_call_expr = std::make_shared<ast::CallExpression>();
    else_console_log_call_expr->callee = else_console_log_member_expr;
    else_console_log_call_expr->arguments = std::vector<std::shared_ptr<ast::Expression>>();
    else_console_log_call_expr->arguments.push_back(its_false_string);

    auto else_expr_statement = std::make_shared<ast::ExpressionStatement>();
    else_expr_statement->expression = else_console_log_call_expr;

    auto else_block_statement = std::make_shared<ast::BlockStatement>();
    else_block_statement->body.push_back(else_expr_statement);

    // if statement
    auto if_statement = std::make_shared<ast::IfStatement>();
    if_statement->test = std::make_shared<ast::BooleanLiteralExpression>("truthy string");
    if_statement->consequent = if_expr_statement;
    if_statement->alternative = else_expr_statement;

    ast::Program program;
    program.body.push_back(if_statement);

    std::cout << program.to_json().dump(4) << "\n";

    // run ast
    interpreter::Interpreter i;
    i.run(program);

    return 0;
}

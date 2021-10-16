#include "interpreter.h"

#include <iostream>

namespace interpreter {

object::Object* Interpreter::execute(std::shared_ptr<ast::Statement> statement) {
    switch (statement->type) {
        case ast::StatementType::Expression: {
            auto s = std::static_pointer_cast<ast::ExpressionStatement>(statement);
            return execute(s->expression);
        }
        case ast::StatementType::If: {
            auto s = std::static_pointer_cast<ast::IfStatement>(statement);

            auto test = execute(s->test);
            if (test->is_truthy()) {
                return execute(s->consequent);
            } else if (s->consequent != nullptr) {
                return execute(s->alternative);
            }

            return object_manager.new_undefined();
        }
        case ast::StatementType::Block: {
            auto s = std::static_pointer_cast<ast::BlockStatement>(statement);
            object::Object* final_value;

            for (auto s: s->body) {
                final_value = execute(s);
            }

            return final_value;
        }
        case ast::StatementType::FunctionDeclaration: {
            auto s = std::static_pointer_cast<ast::FunctionDeclarationStatement>(statement);

            auto func = object_manager.new_function();
            func->is_builtin = false;
            func->body = s->body;

            return set_variable(s->identifier, func);
        }
        case ast::StatementType::VariableDeclaration: {
            // TODO: variable declarations need to run earlier to allow hoisting
            auto s = std::static_pointer_cast<ast::VariableDeclarationStatement>(statement);
            return set_variable(s->identifier, execute(s->value));
        }
    }

    std::cerr << "unable to execute statement type:" << statement->to_json()["type"] << "\n";
    assert(false);
}

object::Object* Interpreter::execute(std::shared_ptr<ast::Expression> expression) {
    switch (expression->type) {
        case ast::ExpressionType::Call: {
            auto e = std::static_pointer_cast<ast::CallExpression>(expression);

            auto func_obj = execute(e->callee);
            assert(func_obj->type() == object::ObjectType::Function);
            auto func = static_cast<object::Function*>(func_obj);

            std::vector<object::Object*> args;

            for (auto arg: e->arguments) {
                args.push_back(execute(arg));
            }

            if (func->is_builtin) {
                return func->builtin_func(args);
            }

            push_scope();

            auto return_value = execute(func->body);

            pop_scope();

            return return_value;
        }
        case ast::ExpressionType::Member: {
            auto e = std::static_pointer_cast<ast::MemberExpression>(expression);

            auto left = std::static_pointer_cast<ast::IdentifierExpression>(e->object);
            assert(left != nullptr);

            auto right = std::static_pointer_cast<ast::IdentifierExpression>(e->property);
            assert(right != nullptr);

            auto obj = get_variable(left->name);

            auto property = obj->get_propery(right->name);

            if (property != nullptr) {
                return property;
            }

            return object_manager.new_undefined();
        }
        case ast::ExpressionType::Identifier: {
            auto e = std::static_pointer_cast<ast::IdentifierExpression>(expression);
            return get_variable(e->name);
        }
        case ast::ExpressionType::NumberLiteral: {
            auto e = std::static_pointer_cast<ast::NumberLiteralExpression>(expression);
            return object_manager.new_number(e->value);
        }
        case ast::ExpressionType::StringLiteral: {
            auto e = std::static_pointer_cast<ast::StringLiteralExpression>(expression);
            return object_manager.new_string(e->value);
        }
        case ast::ExpressionType::BooleanLiteral: {
            auto e = std::static_pointer_cast<ast::BooleanLiteralExpression>(expression);
            return object_manager.new_boolean(e->value);
        }
        case ast::ExpressionType::Binary: {
            auto e = std::static_pointer_cast<ast::BinaryExpression>(expression);
            assert(e->op == ast::Operator::Plus);

            auto right_result = execute(e->right);
            auto left_result = execute(e->left);

            switch (left_result->type()) {
                case object::ObjectType::Number: {
                    auto left = static_cast<object::Number*>(left_result);
                    assert(right_result->type() == object::ObjectType::Number);
                    auto right = static_cast<object::Number*>(right_result);

                    switch (e->op) {
                        case ast::Operator::Plus: {
                            return object_manager.new_number(left->value + right->value);
                        }
                    }

                    assert(false);
                }
                default:
                    assert(false);
            }
        }
    }

    std::cerr << "unable to execute expression type: " << expression->to_json()["type"] << "\n";
    assert(false);
}

void Interpreter::throw_error(std::string type, std::string message) {
    throw Error{type, message};
}

void Interpreter::run(ast::Program &program) {
    object::Object* final_value;

    // TODO: there is probably a way to do this without exceptions but this is quick and easy
    try {
        for (auto s: program.body) {
            final_value = execute(s);
        }
    } catch (Error error) {
        std::cerr << error.type << ": " << error.message;
        return;
    }

    std::cout << final_value->to_string() << "\n";
}

Scope* Interpreter::current_scope() {
    return &scopes[current_scope_index];
}

void Interpreter::push_scope() {
    scopes.push_back(Scope{});
    current_scope_index++;
}

void Interpreter::pop_scope() {
    scopes.pop_back();
    current_scope_index--;
}

object::Object* Interpreter::get_variable(std::string name) {
    auto i = current_scope_index;

    while (i >= 0) {
        auto value = scopes[i].get_variable(name);
        if (value != nullptr) {
            return value;
        }

        i--;
    }

    throw_error("ReferenceError", name + " is not defined");
}

object::Object* Interpreter::set_variable(std::string name, object::Object* value) {
    return current_scope()->set_variable(name, value);
}

Interpreter::Interpreter() {
    // push top level scope
    scopes.push_back(Scope{});

    auto console = object_manager.new_object();

    auto log = object_manager.new_function();
    log->is_builtin = true;
    log->builtin_func = [&](std::vector<object::Object*> args) {
        std::string out;

        for (auto arg: args) {
            out += arg->to_string() + " ";
        }

        std::cout << out << "\n";

        return object_manager.new_undefined();
    };

    console->properties["log"] = log;

    current_scope()->set_variable("console", console);
}

}
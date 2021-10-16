#include "interpreter.h"

#include <iostream>

namespace interpreter {

bool ObjectManager::is_undefined(Object* value) {
    return value == &undefined;
}

Object* ObjectManager::new_object() {
    // just leak for now
    return new Object(ObjectType::Object);
}

Function* ObjectManager::new_function() {
    // just leak for now
    return new Function();
}

Number* ObjectManager::new_number(double value) {
    // just leak for now
    return new Number(value);
}

String* ObjectManager::new_string(std::string value) {
    // just leak for now
    return new String(value);
}

Boolean* ObjectManager::new_boolean(bool value) {
    // just leak for now
    return new Boolean(value);
}

Undefined* ObjectManager::new_undefined() {
    return &undefined;
}

Object* Interpreter::execute(std::shared_ptr<ast::Statement> statement) {
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
            Object* final_value;

            for (auto s: s->body) {
                final_value = execute(s);
            }

            return final_value;
        }
        case ast::StatementType::FunctionDeclaration: {
            auto s = std::static_pointer_cast<ast::FunctionDeclarationStatement>(statement);

            auto name = std::static_pointer_cast<ast::IdentifierExpression>(s->identifier);
            assert(name != nullptr);

            auto func = object_manager.new_function();
            func->is_builtin = false;
            func->body = s->body;

            return set_variable(name->name, func);
        }
        case ast::StatementType::VariableDeclaration: {
            // TODO: variable declarations need to run earlier to allow hoisting

            auto s = std::static_pointer_cast<ast::VariableDeclarationStatement>(statement);

            auto name = std::static_pointer_cast<ast::IdentifierExpression>(s->identifier);
            assert(name != nullptr);

            auto value = execute(s->value);

            return set_variable(name->name, value);
        }
    }

    std::cerr << "unable to execute statement type:" << statement->to_json()["type"] << "\n";
    assert(false);
}

Object* Interpreter::execute(std::shared_ptr<ast::Expression> expression) {
    switch (expression->type) {
        case ast::ExpressionType::Call: {
            auto e = std::static_pointer_cast<ast::CallExpression>(expression);

            auto func_obj = execute(e->callee);
            auto func = static_cast<Function*>(func_obj);
            assert(func != nullptr);

            std::vector<Object*> args;

            for (auto arg: e->arguments) {
                args.push_back(execute(arg));
            }

            if (func->is_builtin) {
                return func->builtin_func(args);
            }
            
            return execute(func->body);
        }
        case ast::ExpressionType::Member: {
            auto e = std::static_pointer_cast<ast::MemberExpression>(expression);

            auto left = std::static_pointer_cast<ast::IdentifierExpression>(e->object);
            assert(left != nullptr);

            auto right = std::static_pointer_cast<ast::IdentifierExpression>(e->property);
            assert(right != nullptr);

            auto obj = get_variable(left->name);
            assert(!object_manager.is_undefined(obj));

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
                case ObjectType::Number: {
                    auto left = static_cast<Number*>(left_result);
                    assert(right_result->type() == ObjectType::Number);
                    auto right = static_cast<Number*>(right_result);

                    switch(e->op) {
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

void Interpreter::run(ast::Program &program) {
    Object* final_value;

    for (auto s: program.body) {
        final_value = execute(s);
    }

    std::cout << final_value->to_string() << "\n";
}

Object* Interpreter::get_variable(std::string name) {
    if (auto entry = variables.find(name); entry != variables.end()) {
        return entry->second;
    }

    return object_manager.new_undefined();
}

Object* Interpreter::set_variable(std::string name, Object* value) {
    variables[name] = value;
    return value;
}

Interpreter::Interpreter() {
    auto console = object_manager.new_object();

    auto log = object_manager.new_function();
    log->is_builtin = true;
    log->builtin_func = [&](std::vector<Object*> args) {
        std::string out;

        for (auto arg: args) {
            out += arg->to_string() + " ";
        }

        std::cout << out << "\n";

        return object_manager.new_undefined();
    };

    console->properties["log"] = log;

    variables["console"] = console;
}

}
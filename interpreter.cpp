#include "interpreter.h"

#include <iostream>

namespace interpreter {

bool ObjectManager::is_undefined(Object* value) {
    return value == &undefined;
}

Object* ObjectManager::new_object() {
    // just leak for now
    return new Object();
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

            assert(func->is_builtin);
            return func->func(args);
        }
        case ast::ExpressionType::Member: {
            auto e = std::static_pointer_cast<ast::MemberExpression>(expression);

            auto left = std::static_pointer_cast<ast::IdentifierExpression>(e->object);
            assert(left != nullptr);

            auto right = std::static_pointer_cast<ast::IdentifierExpression>(e->expression);
            assert(right != nullptr);

            auto obj = lookup_variable(left->name);
            assert(!object_manager.is_undefined(obj));

            auto property = obj->get_propery(right->name);

            if (property != nullptr) {
                return property;
            }

            return object_manager.new_undefined();
        }
        case ast::ExpressionType::Identifier: {
            auto e = std::static_pointer_cast<ast::IdentifierExpression>(expression);
            return object_manager.new_undefined();
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

Object* Interpreter::lookup_variable(std::string name) {
    if (auto entry = variables.find(name); entry != variables.end()) {
        return entry->second;
    }

    return object_manager.new_undefined();
}


Interpreter::Interpreter() {
    auto console = object_manager.new_object();

    auto log = object_manager.new_function();
    log->is_builtin = true;
    log->func = [&](std::vector<Object*> args) {
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
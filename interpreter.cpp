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
        case ast::StatementType::While: {
            auto s = std::static_pointer_cast<ast::WhileStatement>(statement);

            while (execute(s->test)->is_truthy()) {
                execute(s->body);
            }

            return object_manager.new_undefined();
        }
        case ast::StatementType::For: {
            auto s = std::static_pointer_cast<ast::ForStatement>(statement);

            for (execute(s->init); execute(s->test)->is_truthy(); execute(s->update)) {
                execute(s->body);
            }

            return object_manager.new_undefined();
        }
        case ast::StatementType::Block: {
            auto s = std::static_pointer_cast<ast::BlockStatement>(statement);
            object::Object* final_value = nullptr;

            for (auto s: s->body) {
                final_value = execute(s);
            }

            if (final_value == nullptr) {
                return object_manager.new_undefined();
            }

            return final_value;
        }
        case ast::StatementType::FunctionDeclaration: {
            auto s = std::static_pointer_cast<ast::FunctionDeclarationStatement>(statement);

            auto func = object_manager.new_function();
            func->is_builtin = false;
            func->parameters = s->parameters;
            func->body = s->body;

            return declare_variable(s->identifier, func);
        }
        case ast::StatementType::VariableDeclaration: {
            // TODO: variable declarations need to run earlier to allow hoisting
            auto s = std::static_pointer_cast<ast::VariableDeclarationStatement>(statement);
            return declare_variable(s->identifier, execute(s->value));
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

            for (auto i = 0; i < func->parameters.size(); i++) {
                auto has_arg = i < args.size();
                auto value = has_arg ? args[i] : object_manager.new_undefined();
                set_variable(func->parameters[i], value);
            }

            auto return_value = execute(func->body);

            pop_scope();

            return return_value;
        }
        case ast::ExpressionType::Member: {
            auto e = std::static_pointer_cast<ast::MemberExpression>(expression);

            assert(e->object->type == ast::ExpressionType::Identifier);
            auto left = std::static_pointer_cast<ast::IdentifierExpression>(e->object);

            auto obj = get_variable(left->name);


            if (e->is_computed) {
                object::Object* property;
                auto right = execute(e->property);

                if (right->type() == object::ObjectType::Number) {
                    auto index = static_cast<object::Number*>(right);
                    property = obj->get_property(index->value);
                } else {
                    assert(right->type() == object::ObjectType::String);
                    auto name = static_cast<object::String*>(right);
                    property = obj->get_property(name->value);
                }

                if (property != nullptr) {
                    return property;
                }

                return object_manager.new_undefined();
            } else {
                assert(e->property->type == ast::ExpressionType::Identifier);
                auto right = std::static_pointer_cast<ast::IdentifierExpression>(e->property);

                auto property = obj->get_property(right->name);

                if (property != nullptr) {
                    return property;
                }

                return object_manager.new_undefined();
            }

            assert(false);
        }
        case ast::ExpressionType::VariableDeclaration: {
            auto e = std::static_pointer_cast<ast::VariableDeclarationExpression>(expression);
            return declare_variable(e->identifier, execute(e->value));
        }
        case ast::ExpressionType::Assignment: {
            auto e = std::static_pointer_cast<ast::AssignmentExpression>(expression);

            auto right = execute(e->right);

            if (e->left->type == ast::ExpressionType::Identifier) {
                auto left = std::static_pointer_cast<ast::IdentifierExpression>(e->left);
                auto right = execute(e->right);

                if (e->op == ast::Operator::Equals) {
                    return set_variable(left->name, right);
                }

                auto left_number = get_variable(left->name)->as_number();
                auto right_number = right->as_number();

                auto result = object_manager.new_number(0);

                switch (e->op) {
                    case ast::Operator::AdditionAssignment:
                        result->value = left_number->value + right_number->value;
                        break;
                    case ast::Operator::SubtractionAssignment:
                        result->value = left_number->value - right_number->value;
                        break;
                    case ast::Operator::MultiplicationAssignment:
                        result->value = left_number->value * right_number->value;
                        break;
                    case ast::Operator::DivisionAssignment:
                        result->value = left_number->value / right_number->value;
                        break;
                    default:
                        assert(false);
                }

                return set_variable(left->name, result);
            }

            if (e->left->type == ast::ExpressionType::Member) {
                // TODO: handle arithmetic assignments
                assert(e->op == ast::Operator::Equals);
                
                auto left = std::static_pointer_cast<ast::MemberExpression>(e->left);

                if (left->is_computed) {
                    assert(left->object->type == ast::ExpressionType::Identifier);
                    auto object_name = std::static_pointer_cast<ast::IdentifierExpression>(left->object);
                    auto object = get_variable(object_name->name);

                    auto property = execute(left->property);
                    if (property->type() == object::ObjectType::Number) {
                        auto index = static_cast<object::Number*>(property);
                        object->set_property(index->value, right);
                        return property;
                    }

                    if (property->type() == object::ObjectType::String) {
                        auto name = static_cast<object::String*>(property);
                        object->set_property(name->value, right);
                        return property;
                    }

                    assert(false);
                } else {
                    assert(left->object->type == ast::ExpressionType::Identifier);
                    assert(left->property->type == ast::ExpressionType::Identifier);

                    auto left_id = std::static_pointer_cast<ast::IdentifierExpression>(left->object);
                    auto object = get_variable(left_id->name);

                    auto right_id = std::static_pointer_cast<ast::IdentifierExpression>(left->property);

                    object->properties[right_id->name] = right;

                    return right;
                }
            }

            assert(false);
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
        case ast::ExpressionType::Object: {
            auto e = std::static_pointer_cast<ast::ObjectExpression>(expression);
            auto object = object_manager.new_object();

            for (auto p: e->properties) {
                object->properties[p.first] = execute(p.second);
            }

            return object;
        }
        case ast::ExpressionType::Array: {
            auto e = std::static_pointer_cast<ast::ArrayExpression>(expression);
            auto array = object_manager.new_array();

            for (auto e: e->elements) {
                array->elements.push_back(execute(e));
            }

            return array;
        }
        case ast::ExpressionType::Binary: {
            auto e = std::static_pointer_cast<ast::BinaryExpression>(expression);

            auto right_result = execute(e->right);
            auto left_result = execute(e->left);

            switch (left_result->type()) {
                case object::ObjectType::Number: {
                    auto left = static_cast<object::Number*>(left_result);
                    auto right = static_cast<object::Number*>(right_result);

                    switch (e->op) {
                        case ast::Operator::Plus: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_number(left->value + right->value);
                        }
                        case ast::Operator::Minus: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_number(left->value - right->value);
                        }
                        case ast::Operator::Multiply: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_number(left->value * right->value);
                        }
                        case ast::Operator::Divide: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_number(left->value / right->value);
                        }
                        case ast::Operator::Modulo: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_number(std::fmod(left->value, right->value));
                        }
                        case ast::Operator::EqualTo: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value == right->value);
                        }
                        case ast::Operator::EqualToStrict: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value == right->value);
                        }
                        case ast::Operator::And: {
                            return object_manager.new_boolean(left->is_truthy() && right->is_truthy());
                        }
                        case ast::Operator::Or: {
                            return object_manager.new_boolean(left->is_truthy() || right->is_truthy());
                        }
                        case ast::Operator::NotEqualTo: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value != right->value);
                        }
                        case ast::Operator::GreaterThan: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value > right->value);
                        }
                        case ast::Operator::GreaterThanOrEqualTo: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value >= right->value);
                        }
                        case ast::Operator::LessThan: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value < right->value);
                        }
                        case ast::Operator::LessThanOrEqualTo: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_boolean(left->value <= right->value);
                        }
                        case ast::Operator::Equals:
                        case ast::Operator::Increment:
                        case ast::Operator::Decrement: {
                            assert(false);
                        }
                    }

                    assert(false);
                }
                case object::ObjectType::Boolean: {
                    auto left = static_cast<object::Boolean*>(left_result);

                    switch (e->op) {
                        case ast::Operator::EqualTo: {
                            return object_manager.new_boolean(left->value == right_result->is_truthy());
                        }
                        case ast::Operator::EqualToStrict: {
                            return object_manager.new_boolean(left->value == right_result->is_truthy());
                        }
                        case ast::Operator::And: {
                            return object_manager.new_boolean(left->value && right_result->is_truthy());
                        }
                        case ast::Operator::Or: {
                            return object_manager.new_boolean(left->value || right_result->is_truthy());
                        }
                        case ast::Operator::NotEqualTo: {
                            return object_manager.new_boolean(left->value != right_result->is_truthy());
                        }
                        case ast::Operator::GreaterThan: {
                            return object_manager.new_boolean(left->value > right_result->is_truthy());
                        }
                        case ast::Operator::GreaterThanOrEqualTo: {
                            return object_manager.new_boolean(left->value >= right_result->is_truthy());
                        }
                        case ast::Operator::LessThan: {
                            return object_manager.new_boolean(left->value < right_result->is_truthy());
                        }
                        case ast::Operator::LessThanOrEqualTo: {
                            return object_manager.new_boolean(left->value <= right_result->is_truthy());
                        }
                        case ast::Operator::Plus:
                        case ast::Operator::Minus:
                        case ast::Operator::Multiply:
                        case ast::Operator::Divide:
                        case ast::Operator::Modulo:
                        case ast::Operator::Equals:
                        case ast::Operator::Increment:
                        case ast::Operator::Decrement: {
                            assert(false);
                        }
                    }
                }
                default:
                    assert(false);
            }
        }
        case ast::ExpressionType::Update: {
            auto e = std::static_pointer_cast<ast::UpdateExpression>(expression);
            assert(e->op == ast::Operator::Increment || e->op == ast::Operator::Decrement);
            assert(e->argument->type == ast::ExpressionType::Identifier);

            auto identifier = std::static_pointer_cast<ast::IdentifierExpression>(e->argument);
            auto value_object = get_variable(identifier->name);
            assert(value_object->type() == object::ObjectType::Number);
            auto value_number = static_cast<object::Number*>(value_object);
            auto new_value = e->op == ast::Operator::Increment ? value_number->value + 1 : value_number->value - 1;

            set_variable(identifier->name, object_manager.new_number(new_value));

            return object_manager.new_number(e->is_prefix ? new_value : value_number->value);
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
    assert(false);
}

object::Object* Interpreter::declare_variable(std::string name, object::Object* value) {
    return current_scope()->set_variable(name, value);
}

object::Object* Interpreter::set_variable(std::string name, object::Object* value) {
    auto var = current_scope()->get_variable(name);

    // undeclared so set in top level scope, closest thing we have to "global" right now
    if (var == nullptr) {
        return scopes[0].set_variable(name, value);
    }

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
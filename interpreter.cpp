#include "interpreter.h"

#include <iostream>

namespace interpreter {

object::Object* Interpreter::execute(std::shared_ptr<ast::Statement> statement) {
    switch (statement->type) {
        case ast::StatementType::Expression: {
            auto s = statement->as_expression_statement();
            return execute(s->expression);
        }
        case ast::StatementType::If: {
            auto s = statement->as_if();

            auto test = execute(s->test);
            if (test->is_truthy()) {
                return execute(s->consequent);
            } else if (s->alternative != nullptr) {
                return execute(s->alternative);
            }

            return object_manager.new_undefined();
        }
        case ast::StatementType::While: {
            auto s = statement->as_while();

            while (execute(s->test)->is_truthy()) {
                execute(s->body);
            }

            return object_manager.new_undefined();
        }
        case ast::StatementType::For: {
            auto s = statement->as_for();

            for (execute(s->init); execute(s->test)->is_truthy(); execute(s->update)) {
                execute(s->body);
            }

            return object_manager.new_undefined();
        }
        case ast::StatementType::Block: {
            auto s = statement->as_block();
            object::Object* final_value = nullptr;

            for (auto s: s->body) {
                final_value = execute(s);
                if (s->type == ast::StatementType::Return) {
                    return final_value;
                }
            }

            if (final_value == nullptr) {
                return object_manager.new_undefined();
            }

            return final_value;
        }
        case ast::StatementType::FunctionDeclaration: {
            auto s = statement->as_function_declaration();

            auto func = object_manager.new_function();
            func->is_builtin = false;
            func->parameters = s->parameters;
            func->body = s->body;

            return declare_variable(s->identifier, func);
        }
        case ast::StatementType::Return: {
            auto s = statement->as_return();
            if (s->argument == nullptr) {
                return object_manager.new_undefined();
            }

            return execute(s->argument);
        }
    }

    std::cerr << "unable to execute statement type:" << statement->to_json()["type"] << "\n";
    assert(false);
}

object::Object* Interpreter::execute(std::shared_ptr<ast::Expression> expression) {
    switch (expression->type) {
        case ast::ExpressionType::Call: {
            auto e = expression->as_call();

            auto func_obj = execute(e->callee);
            if (func_obj->type() == object::ObjectType::Undefined) {
                assert(e->callee->type == ast::ExpressionType::Member);
                auto callee = e->callee->as_member();
                assert(callee->object->type == ast::ExpressionType::Identifier);
                assert(callee->property->type == ast::ExpressionType::Identifier);
                assert(callee->is_computed == false);

                auto object_id = callee->object->as_identifier()->name;
                auto property_id = callee->property->as_identifier()->name;
                throw_error("TypeError", object_id + "." + property_id + " is not a function");
                assert(false);
            }

            assert(func_obj->type() == object::ObjectType::Function);
            auto func = static_cast<object::Function*>(func_obj);

            std::vector<object::Object*> args;

            for (auto arg: e->arguments) {
                args.push_back(execute(arg));
            }

            if (func->is_builtin) {
                return func->builtin_func(args);
            }

            object_manager.push_scope();

            for (auto i = 0; i < func->parameters.size(); i++) {
                auto has_arg = i < args.size();
                auto value = has_arg ? args[i] : object_manager.new_undefined();
                set_variable(func->parameters[i], value);
            }

            auto return_value = execute(func->body);

            object_manager.pop_scope();

            return return_value;
        }
        case ast::ExpressionType::Member: {
            auto e = expression->as_member();

            auto left = e->object->as_identifier();
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
                auto right = e->property->as_identifier();

                auto property = obj->get_property(right->name);

                if (property != nullptr) {
                    return property;
                }

                return object_manager.new_undefined();
            }

            assert(false);
        }
        case ast::ExpressionType::VariableDeclaration: {
            auto e = expression->as_variable_declaration();
            object::Object* value = e->value != nullptr ? execute(e->value) : object_manager.new_undefined();
            for (auto id: e->identifiers) {
                declare_variable(id, value);
            }
            return value;
        }
        case ast::ExpressionType::Assignment: {
            auto e = expression->as_assignment();

            auto right = execute(e->right);

            if (e->left->type == ast::ExpressionType::Identifier) {
                auto left = e->left->as_identifier();
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

                auto left = e->left->as_member();

                if (left->is_computed) {
                    auto object_name = left->object->as_identifier();
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
                    auto left_id = left->object->as_identifier();
                    auto object = get_variable(left_id->name);

                    auto right_id = left->property->as_identifier();

                    object->properties[right_id->name] = right;

                    return right;
                }
            }

            assert(false);
        }
        case ast::ExpressionType::Identifier: {
            return get_variable(expression->as_identifier()->name);
        }
        case ast::ExpressionType::NumberLiteral: {
            return object_manager.new_number(expression->as_number_literal()->value);
        }
        case ast::ExpressionType::StringLiteral: {
            return object_manager.new_string(expression->as_string_literal()->value);
        }
        case ast::ExpressionType::BooleanLiteral: {
            return object_manager.new_boolean(expression->as_boolean_literal()->value);
        }
        case ast::ExpressionType::Object: {
            auto e = expression->as_object();
            auto object = object_manager.new_object();

            for (auto p: e->properties) {
                object->properties[p.first] = execute(p.second);
            }

            return object;
        }
        case ast::ExpressionType::Array: {
            auto e = expression->as_array();
            auto array = object_manager.new_array();

            for (auto e: e->elements) {
                array->elements.push_back(execute(e));
            }

            return array;
        }
        case ast::ExpressionType::Binary: {
            auto e = expression->as_binary();

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
                        case ast::Operator::Exponentiation: {
                            assert(right_result->type() == object::ObjectType::Number);
                            return object_manager.new_number(std::powf(left->value, right->value));
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
                        case ast::Operator::NotEqualToStrict: {
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
                        case ast::Operator::AdditionAssignment:
                        case ast::Operator::SubtractionAssignment:
                        case ast::Operator::MultiplicationAssignment:
                        case ast::Operator::DivisionAssignment:
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
                        case ast::Operator::NotEqualToStrict: {
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
                        case ast::Operator::AdditionAssignment:
                        case ast::Operator::SubtractionAssignment:
                        case ast::Operator::MultiplicationAssignment:
                        case ast::Operator::DivisionAssignment:
                        case ast::Operator::Increment:
                        case ast::Operator::Decrement:
                        case ast::Operator::Exponentiation: {
                            assert(false);
                        }
                    }
                }
                default:
                    assert(false);
            }
        }
        case ast::ExpressionType::Update: {
            auto e = expression->as_update();
            assert(e->op == ast::Operator::Increment || e->op == ast::Operator::Decrement);
            assert(e->argument->type == ast::ExpressionType::Identifier);

            auto identifier = e->argument->as_identifier();
            auto value_object = get_variable(identifier->name);
            assert(value_object->type() == object::ObjectType::Number);
            auto value_number = static_cast<object::Number*>(value_object);
            auto new_value = e->op == ast::Operator::Increment ? value_number->value + 1 : value_number->value - 1;

            set_variable(identifier->name, object_manager.new_number(new_value));

            return object_manager.new_number(e->is_prefix ? new_value : value_number->value);
        }
        case ast::ExpressionType::Ternary: {
            auto e = expression->as_ternary();

            if (execute(e->test)->is_truthy()) {
                return execute(e->consequent);
            } else {
                return execute(e->alternative);
            }
        }
    }

    std::cerr << "unable to execute expression type: " << expression->
            to_json()["type"]
              << "\n";
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

}

object::Object* Interpreter::get_variable(std::string name) {
    auto v = object_manager.get_variable(name);
    if (v == nullptr) {
        throw_error("ReferenceError", name + " is not defined");
        assert(false);
    }

    return v;
}

object::Object* Interpreter::declare_variable(std::string name, object::Object* value) {
    return object_manager.set_variable(name, value);
}

object::Object* Interpreter::set_variable(std::string name, object::Object* value) {
    return object_manager.set_variable(name, value);
}

Interpreter::Interpreter() {
    auto console = object_manager.new_object();
    console->register_native_method("log", [&](std::vector<object::Object*> args) {
        std::string out;

        for (auto arg: args) {
            out += arg->to_string() + " ";
        }

        std::cout << out << "\n";

        return object_manager.new_undefined();
    });

    object_manager.current_scope()->set_variable("console", console);

    auto Math = object_manager.new_object();
    Math->register_native_method("abs", [&](std::vector<object::Object*> args) {
        auto arg = args[0]->as_number();
        return object_manager.new_number(std::fabs(arg->value));
    });
    Math->register_native_method("round", [&](std::vector<object::Object*> args) {
        auto arg = args[0]->as_number();
        return object_manager.new_number(std::roundf(arg->value));
    });
    Math->register_native_method("sqrt", [&](std::vector<object::Object*> args) {
        auto arg = args[0]->as_number();
        return object_manager.new_number(std::sqrtf(arg->value));
    });

    object_manager.current_scope()->set_variable("Math", Math);
}

}
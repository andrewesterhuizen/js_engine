#include "interpreter.h"

#include <iostream>

namespace interpreter {

object::Value* Interpreter::execute(std::shared_ptr<ast::Statement> statement) {
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

            return om.new_undefined();
        }
        case ast::StatementType::While: {
            auto s = statement->as_while();

            while (execute(s->test)->is_truthy()) {
                execute(s->body);
            }

            return om.new_undefined();
        }
        case ast::StatementType::For: {
            auto s = statement->as_for();

            for (execute(s->init); execute(s->test)->is_truthy(); execute(s->update)) {
                execute(s->body);
            }

            return om.new_undefined();
        }
        case ast::StatementType::Block: {
            auto s = statement->as_block();
            object::Value* final_value = nullptr;

            for (auto s: s->body) {
                final_value = execute(s);
                if (s->type == ast::StatementType::Return) {
                    return final_value;
                }
            }

            if (final_value == nullptr) {
                return om.new_undefined();
            }

            return final_value;
        }
        case ast::StatementType::FunctionDeclaration: {
            auto s = statement->as_function_declaration();

            auto func_value = om.new_function();
            auto func = func_value->function();
            func->is_builtin = false;
            func->parameters = s->parameters;
            func->body = s->body;

            return declare_variable(s->identifier, func_value);
        }
        case ast::StatementType::Return: {
            auto s = statement->as_return();
            if (s->argument == nullptr) {
                return om.new_undefined();
            }

            return execute(s->argument);
        }
    }

    std::cerr << "unable to execute statement type:" << statement->to_json()["type"] << "\n";
    assert(false);
}

object::Value* Interpreter::execute(std::shared_ptr<ast::Expression> expression) {
    switch (expression->type) {
        case ast::ExpressionType::Call: {
            auto e = expression->as_call();

            auto func_obj = execute(e->callee);
            if (func_obj->type == object::Value::Type::Undefined) {
                if (e->callee->type == ast::ExpressionType::Identifier) {
                    auto callee = e->callee->as_identifier();
                    throw_error("TypeError", callee->name + " is not a function");
                } else if (e->callee->type == ast::ExpressionType::Member) {
                    auto callee = e->callee->as_member();
                    assert(callee->object->type == ast::ExpressionType::Identifier);
                    assert(callee->property->type == ast::ExpressionType::Identifier);
                    assert(callee->is_computed == false);

                    auto object_id = callee->object->as_identifier()->name;
                    auto property_id = callee->property->as_identifier()->name;
                    throw_error("TypeError", object_id + "." + property_id + " is not a function");
                }

                assert(false);
            }

            std::vector<object::Value*> args;

            for (auto arg: e->arguments) {
                args.push_back(execute(arg));
            }

            if (e->callee->type == ast::ExpressionType::Member) {
                auto object = execute(e->callee->as_member()->object);
                return call_function(object, func_obj, args);
            }

            // TODO: pass global this context
            return call_function(nullptr, func_obj, args);
        }
        case ast::ExpressionType::Member: {
            auto e = expression->as_member();
            auto obj = execute(e->object);

            if (e->is_computed) {
                object::Value* property;
                auto right = execute(e->property);

                if (right->type == object::Value::Type::Number) {
                    property = obj->get_property(om, right->number());
                } else {
                    assert(right->type == object::Value::Type::String);
                    property = obj->get_property(om, right->string());
                }

                if (property != nullptr) {
                    return property;
                }

                return om.new_undefined();
            } else {
                auto right = e->property->as_identifier();

                auto property = obj->get_property(om, right->name);

                if (property != nullptr) {
                    return property;
                }

                return om.new_undefined();
            }

            assert(false);
        }
        case ast::ExpressionType::VariableDeclaration: {
            auto e = expression->as_variable_declaration();
            object::Value* value = e->value != nullptr ? execute(e->value) : om.new_undefined();
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

                auto left_number = get_variable(left->name)->number();
                auto right_number = right->number();

                auto result = om.new_number(0);

                switch (e->op) {
                    case ast::Operator::AdditionAssignment:
                        result->value = left_number + right_number;
                        break;
                    case ast::Operator::SubtractionAssignment:
                        result->value = left_number - right_number;
                        break;
                    case ast::Operator::MultiplicationAssignment:
                        result->value = left_number * right_number;
                        break;
                    case ast::Operator::DivisionAssignment:
                        result->value = left_number / right_number;
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
                    if (property->type == object::Value::Type::Number) {
                        object->set_property(property->number(), right);
                        return property;
                    }

                    if (property->type == object::Value::Type::String) {
                        object->set_property(property->string(), right);
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
            return om.new_number(expression->as_number_literal()->value);
        }
        case ast::ExpressionType::StringLiteral: {
            return om.new_string(expression->as_string_literal()->value);
        }
        case ast::ExpressionType::BooleanLiteral: {
            return om.new_boolean(expression->as_boolean_literal()->value);
        }
        case ast::ExpressionType::Object: {
            auto e = expression->as_object();
            auto object = om.new_object();

            for (auto p: e->properties) {
                object->properties[p.first] = execute(p.second);
            }

            return object;
        }
        case ast::ExpressionType::Array: {
            auto e = expression->as_array();
            auto array_value = om.new_array();
            auto array = array_value->array();

            for (auto e: e->elements) {
                array->elements.push_back(execute(e));
            }

            return array_value;
        }
        case ast::ExpressionType::Function: {
            auto e = expression->as_function();

            auto func_value = om.new_function();
            auto func = func_value->function();
            func->is_builtin = false;
            func->parameters = e->parameters;
            func->body = e->body;

            return func_value;
        }
        case ast::ExpressionType::ArrowFunction: {
            auto e = expression->as_arrow_function();

            auto func_value = om.new_function();
            auto func = func_value->function();
            func->is_builtin = false;
            func->parameters = e->parameters;
            func->body = e->body;

            return func_value;
        }
        case ast::ExpressionType::Binary: {
            auto e = expression->as_binary();

            auto right_result = execute(e->right);
            auto left_result = execute(e->left);

            switch (left_result->type) {
                case object::Value::Type::Number: {
                    switch (e->op) {
                        case ast::Operator::Plus: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_number(left_result->number() + right_result->number());
                        }
                        case ast::Operator::Minus: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_number(left_result->number() - right_result->number());
                        }
                        case ast::Operator::Multiply: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_number(left_result->number() * right_result->number());
                        }
                        case ast::Operator::Divide: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_number(left_result->number() / right_result->number());
                        }
                        case ast::Operator::Modulo: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_number(std::fmod(left_result->number(), right_result->number()));
                        }
                        case ast::Operator::Exponentiation: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_number(std::powf(left_result->number(), right_result->number()));
                        }
                        case ast::Operator::EqualTo: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() == right_result->number());
                        }
                        case ast::Operator::EqualToStrict: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() == right_result->number());
                        }
                        case ast::Operator::And: {
                            return om.new_boolean(left_result->is_truthy() && right_result->is_truthy());
                        }
                        case ast::Operator::Or: {
                            return om.new_boolean(left_result->is_truthy() || right_result->is_truthy());
                        }
                        case ast::Operator::NotEqualTo: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() != right_result->number());
                        }
                        case ast::Operator::NotEqualToStrict: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() != right_result->number());
                        }
                        case ast::Operator::GreaterThan: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() > right_result->number());
                        }
                        case ast::Operator::GreaterThanOrEqualTo: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() >= right_result->number());
                        }
                        case ast::Operator::LessThan: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() < right_result->number());
                        }
                        case ast::Operator::LessThanOrEqualTo: {
                            assert(right_result->type == object::Value::Type::Number);
                            return om.new_boolean(left_result->number() <= right_result->number());
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
                case object::Value::Type::String:
                case object::Value::Type::Object:
                case object::Value::Type::Array:
                case object::Value::Type::Function:
                case object::Value::Type::Undefined:
                case object::Value::Type::Boolean: {
                    switch (e->op) {
                        case ast::Operator::EqualTo: {
                            return om.new_boolean(left_result->is_truthy() == right_result->is_truthy());
                        }
                        case ast::Operator::EqualToStrict: {
                            return om.new_boolean(left_result->is_truthy() == right_result->is_truthy());
                        }
                        case ast::Operator::And: {
                            return om.new_boolean(left_result->is_truthy() && right_result->is_truthy());
                        }
                        case ast::Operator::Or: {
                            return om.new_boolean(left_result->is_truthy() || right_result->is_truthy());
                        }
                        case ast::Operator::NotEqualTo: {
                            return om.new_boolean(left_result->is_truthy() != right_result->is_truthy());
                        }
                        case ast::Operator::NotEqualToStrict: {
                            return om.new_boolean(left_result->is_truthy() != right_result->is_truthy());
                        }
                        case ast::Operator::GreaterThan: {
                            return om.new_boolean(left_result->is_truthy() > right_result->is_truthy());
                        }
                        case ast::Operator::GreaterThanOrEqualTo: {
                            return om.new_boolean(left_result->is_truthy() >= right_result->is_truthy());
                        }
                        case ast::Operator::LessThan: {
                            return om.new_boolean(left_result->is_truthy() < right_result->is_truthy());
                        }
                        case ast::Operator::LessThanOrEqualTo: {
                            return om.new_boolean(left_result->is_truthy() <= right_result->is_truthy());
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
            }

            assert(false);
        }
        case ast::ExpressionType::Update: {
            auto e = expression->as_update();
            assert(e->op == ast::Operator::Increment || e->op == ast::Operator::Decrement);
            assert(e->argument->type == ast::ExpressionType::Identifier);

            auto identifier = e->argument->as_identifier();
            auto value_object = get_variable(identifier->name);
            assert(value_object->type == object::Value::Type::Number);
            auto new_value =
                    e->op == ast::Operator::Increment ? value_object->number() + 1 : value_object->number() - 1;

            set_variable(identifier->name, om.new_number(new_value));

            return om.new_number(e->is_prefix ? new_value : value_object->number());
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

    std::cerr << "unable to execute expression type: " << expression->to_json()["type"]
              << "\n";
    assert(false);
}

object::Value*
Interpreter::call_function(object::Value* this_context, object::Value* func_value, std::vector<object::Value*> args) {
    assert(func_value->type == object::Value::Type::Function);

    auto func = func_value->function();

    if (func->is_builtin) {
        return func->builtin_func(this_context, args);
    }

    om.push_scope();

    for (auto i = 0; i < func->parameters.size(); i++) {
        auto has_arg = i < args.size();
        auto value = has_arg ? args[i] : om.new_undefined();
        set_variable(func->parameters[i], value);
    }

    auto return_value = execute(func->body);

    om.pop_scope();

    return return_value;
}

void Interpreter::throw_error(std::string type, std::string message) {
    throw Error{type, message};
}

void Interpreter::run(ast::Program &program) {
    object::Value* final_value;

    // TODO: there is probably a way to do this without exceptions but this is quick and easy
    try {
        for (auto s: program.body) {
            final_value = execute(s);
        }
    }
    catch (Error error) {
        std::cerr << error.type << ": " << error.message;
        return;
    }
}

object::Value* Interpreter::get_variable(std::string name) {
    auto v = om.get_variable(name);
    if (v == nullptr) {
        throw_error("ReferenceError", name + " is not defined");
        assert(false);
    }

    return v;
}

object::Value* Interpreter::declare_variable(std::string name, object::Value* value) {
    return om.set_variable(name, value);
}

object::Value* Interpreter::set_variable(std::string name, object::Value* value) {
    return om.set_variable(name, value);
}

void Interpreter::create_builtin_objects() {
    // console
    auto console = om.new_object();
    om.current_scope()->set_variable("console", console);

    console->register_native_method(om, "log", [&](object::Value*, std::vector<object::Value*> args) {
        std::string out;

        for (auto arg: args) {
            out += arg->to_string() + " ";
        }

        std::cout << out << "\n";

        return om.new_undefined();
    });

    // Math
    auto Math = om.new_object();
    om.current_scope()->set_variable("Math", Math);
    Math->register_native_method(om, "abs", [&](object::Value*, std::vector<object::Value*> args) {
        return om.new_number(std::fabs(args[0]->number()));
    });
    Math->register_native_method(om, "round", [&](object::Value*, std::vector<object::Value*> args) {
        return om.new_number(std::roundf(args[0]->number()));
    });
    Math->register_native_method(om, "sqrt", [&](object::Value*, std::vector<object::Value*> args) {
        return om.new_number(std::sqrtf(args[0]->number()));
    });

    // Array
    auto Array = om.new_object();
    om.current_scope()->set_variable("Array", Array);

    Array->register_native_method(om, "push", [&](object::Value* this_context, std::vector<object::Value*> args) {
        auto array = this_context->array();

        for (auto arg: args) {
            array->elements.push_back(arg);
        }

        return this_context->get_property(om, "length");
    });

    Array->register_native_method(om, "pop", [&](object::Value* this_context, std::vector<object::Value*> args) {
        auto array = this_context->array();

        if (array->elements.size() == 0) {
            return om.new_undefined();
        }

        auto value = array->elements[array->elements.size() - 1];
        array->elements.pop_back();
        return value;
    });

    Array->register_native_method(om, "forEach", [&](object::Value* this_context, std::vector<object::Value*> args) {
        auto caller_obj = this_context->array();
        auto callback = args[0];

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{caller_obj->elements[i], om.new_number(i)};
            call_function(this_context, callback, args);
        }

        return om.new_undefined();
    });

    Array->register_native_method(om, "map", [&](object::Value* this_context, std::vector<object::Value*> args) {
        auto caller_obj = this_context->array();
        auto callback = args[0];

        auto result_value = om.new_array();
        auto result = result_value->array();

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{caller_obj->elements[i], om.new_number(i)};
            auto v = call_function(this_context, callback, args);
            result->elements.push_back(v);
        }

        return result_value;
    });

    Array->register_native_method(om, "filter", [&](object::Value* this_context, std::vector<object::Value*> args) {
        auto caller_obj = this_context->array();
        auto callback = args[0];

        auto result_value = om.new_array();
        auto result = result_value->array();

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{caller_obj->elements[i], om.new_number(i)};
            auto v = call_function(this_context, callback, args);
            if (v->is_truthy()) {
                result->elements.push_back(v);
            }
        }

        return result_value;
    });

    Array->register_native_method(om, "reduce", [&](object::Value* this_context, std::vector<object::Value*> args) {
        auto caller_obj = this_context->array();
        auto callback = args[0];
        auto initial_value = args[1];

        auto prev = initial_value;

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{prev, caller_obj->elements[i], om.new_number(i)};
            prev = call_function(this_context, callback, args);
        }

        return prev;
    });
}

Interpreter::Interpreter() {
    create_builtin_objects();
}

}
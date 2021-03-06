#include "interpreter.h"

#include <iostream>

namespace interpreter {

object::Value* Interpreter::execute(std::shared_ptr<ast::Statement> statement) {
    switch (statement->type) {
        case ast::StatementType::Throw: {
            auto s = statement->as_throw();
            auto arg = execute(s->argument);
            throw arg;
        }
        case ast::StatementType::TryCatch: {
            auto s = statement->as_trycatch();
            try {
                return execute(s->try_body);
            } catch (object::Value* error) {
                om.push_scope(om.new_object());
                om.current_scope()->set_variable(s->catch_identifier, error);
                execute(s->catch_body);
                om.pop_scope();
                return om.new_undefined();
            }
        }
        case ast::StatementType::Expression: {
            auto s = statement->as_expression_statement();
            return execute(s->expression);
        }
        case ast::StatementType::If: {
            auto s = statement->as_if();

            auto test = execute(s->test);
            if (test->is_truthy()) {
                return execute(s->consequent);
            } else if (s->alternative.has_value()) {
                return execute(s->alternative.value());
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
            auto block = statement->as_block();

            for (auto s: block->body) {
                execute(s);
            }

            return om.new_undefined();
        }
        case ast::StatementType::FunctionDeclaration: {
            auto s = statement->as_function_declaration();

            auto func_value = om.new_function(s->identifier);
            auto func = func_value->function();
            func->is_builtin = false;
            func->parameters = s->parameters;
            func->body = s->body;

            return declare_variable(s->identifier, func_value);
        }
        case ast::StatementType::Return: {
            auto s = statement->as_return();

            if (!s->argument.has_value()) {
                throw Return{om.new_undefined()};
            }

            auto value = execute(s->argument.value());
            throw Return{value};
        }
    }

    std::cerr << "unable to execute statement type:" << statement->to_json()["type"] << "\n";
    assert(false);
}

object::Value* Interpreter::execute(std::shared_ptr<ast::Expression> expression) {
    switch (expression->type) {
        case ast::ExpressionType::New: {
            auto e = expression->as_new();
            auto constructor = execute(e->callee);
            assert(constructor->type == object::Value::Type::Function);

            auto instance = om.new_object();

            auto prototype = constructor->get_property(om, "prototype");
            assert(prototype.has_value());
            instance->set_property("__proto__", prototype.value());

            std::vector<object::Value*> args;
            for (auto arg: e->arguments) {
                args.push_back(execute(arg));
            }

            auto result = call_function(instance, constructor, args);
            if (!result->is_undefined()) {
                return result;
            }

            return instance;
        }

        case ast::ExpressionType::This: {
            return om.current_scope()->this_context();
        }
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

            return call_function(om.global_object(), func_obj, args);
        }
        case ast::ExpressionType::Member: {
            auto e = expression->as_member();
            auto obj = execute(e->object);

            if (e->is_computed) {
                std::optional<object::Value*> property;
                auto right = execute(e->property);

                if (right->type == object::Value::Type::Number) {
                    property = obj->get_property(om, right->number());
                } else {
                    assert(right->type == object::Value::Type::String);
                    property = obj->get_property(om, right->string());
                }

                if (property.has_value()) {
                    return property.value();
                }

                return om.new_undefined();
            } else {
                auto right = e->property->as_identifier();

                auto property = obj->get_property(om, right->name);

                if (property.has_value()) {
                    return property.value();
                }

                return om.new_undefined();
            }

            assert(false);
        }
        case ast::ExpressionType::VariableDeclaration: {
            auto e = expression->as_variable_declaration();
            object::Value* value = e->value.has_value() ? execute(e->value.value()) : om.new_undefined();
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
                    auto object = execute(left->object);

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
                    auto object = execute(left->object);
                    auto property = left->property->as_identifier();
                    return object->set_property(property->name, right);
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
        case ast::ExpressionType::NullLiteral: {
            return om.new_null();
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
                            if (right_result->type == object::Value::Type::String) {
                                return om.new_string(left_result->to_string() + right_result->string());
                            }

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
                            if (right_result->type == object::Value::Type::Number) {
                                return om.new_boolean(left_result->number() == right_result->number());
                            }

                            // TODO: this should attempt conversion to number for non number values
                            return om.new_boolean(false);
                        }
                        case ast::Operator::EqualToStrict: {
                            if (right_result->type == object::Value::Type::Number) {
                                return om.new_boolean(left_result->number() == right_result->number());
                            }

                            return om.new_boolean(false);
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
                        case ast::Operator::BitwiseAnd: {
                            auto left_int = static_cast<int>(left_result->number());
                            auto right_int = static_cast<int>(right_result->number());
                            return om.new_number(left_int & right_int);
                        }
                        case ast::Operator::BitwiseOr: {
                            auto left_int = static_cast<int>(left_result->number());
                            auto right_int = static_cast<int>(right_result->number());
                            return om.new_number(left_int | right_int);
                        }
                        case ast::Operator::Equals:
                        case ast::Operator::AdditionAssignment:
                        case ast::Operator::SubtractionAssignment:
                        case ast::Operator::MultiplicationAssignment:
                        case ast::Operator::DivisionAssignment:
                        case ast::Operator::Increment:
                        case ast::Operator::Decrement:
                        case ast::Operator::Not:
                        case ast::Operator::Typeof: {
                            assert(false);
                        }
                    }

                    assert(false);
                }
                case object::Value::Type::String: {
                    if (e->op == ast::Operator::Plus) {
                        if (right_result->type == object::Value::Type::String) {
                            return om.new_string(left_result->string() + right_result->string());
                        }

                        return om.new_string(left_result->string() + right_result->to_string());
                    }
                    // intentionally fall through here
                }
                case object::Value::Type::Object:
                case object::Value::Type::Array:
                case object::Value::Type::Function:
                case object::Value::Type::Undefined:
                case object::Value::Type::Null:
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
                            if (right_result->type == object::Value::Type::String) {
                                return om.new_string(left_result->to_string() + right_result->string());
                            }

                            return om.new_string(left_result->to_string() + right_result->to_string());
                        case ast::Operator::BitwiseAnd:
                        case ast::Operator::BitwiseOr:
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
                        case ast::Operator::Exponentiation:
                        case ast::Operator::Not:
                        case ast::Operator::Typeof: {
                            assert(false);
                        }
                    }
                }
            }

            assert(false);
        }
        case ast::ExpressionType::Unary: {
            auto e = expression->as_unary();
            switch (e->op) {
                case ast::Operator::Not:
                    return om.new_boolean(!execute(e->argument)->is_truthy());
                case ast::Operator::Typeof: {
                    auto result = execute(e->argument);
                    return om.new_string(result->type_of());
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
Interpreter::call_function(object::Value* context, object::Value* func_value, std::vector<object::Value*> args) {
    assert(func_value->type == object::Value::Type::Function);

    auto func = func_value->function();

    if (func->is_builtin) {
        return func->builtin_func(context, args);
    }

    // construct "arguments" object
    om.push_scope(context);
    auto arguments_object = om.new_array();

    for (auto arg: args) {
        arguments_object->array()->elements.push_back(arg);
    }

    set_variable("arguments", arguments_object);

    // assign arguments to function parameters
    for (auto i = 0; i < func->parameters.size(); i++) {
        auto has_arg = i < args.size();
        auto value = has_arg ? args[i] : om.new_undefined();
        set_variable(func->parameters[i], value);
    }

    auto return_value = om.new_undefined();

    if (func->body->type == ast::ASTNodeType::Statement) {
        try {
            execute(std::static_pointer_cast<ast::Statement>(func->body));
        } catch (Return ret) {
            return_value = ret.value;
        }
    } else {
        return_value = execute(std::static_pointer_cast<ast::Expression>(func->body));
    }

    om.pop_scope();

    return return_value;
}

void Interpreter::throw_error(std::string error_type, std::string message) {
    auto error_constructor_value = get_variable(error_type);
    auto error_instance = call_function(om.new_object(), error_constructor_value, {om.new_string(message)});

    auto prototype = error_constructor_value->get_property(om, "prototype");
    assert(prototype.has_value());
    error_instance->set_property("__proto__", prototype.value());

    throw error_instance;
}

void Interpreter::run(ast::Program &program) {
    try {
        for (auto s: program.body) {
            execute(s);
        }
    }
    catch (object::Value* error) {
        auto error_to_string = error->get_property(om, "toString");
        auto string_value = call_function(error, error_to_string.value(), {});
        std::cerr << string_value->string() << "\n";
        return;
    }
}

object::Value* Interpreter::get_variable(std::string name) {
    auto v = om.get_variable(name);
    if (!v.has_value()) {
        throw_error("ReferenceError", name + " is not defined\n");
        assert(false);
    }

    return v.value();
}

object::Value* Interpreter::declare_variable(std::string name, object::Value* value) {
    return om.set_variable(name, value);
}

object::Value* Interpreter::set_variable(std::string name, object::Value* value) {
    return om.set_variable(name, value);
}

void Interpreter::create_builtin_objects() {
    auto global = om.global_object();

    // Built in prototypes
    auto object_prototype = om.new_object();
    auto proto = new object::Value{};
    proto->type = object::Value::Type::Undefined;
    object_prototype->set_property("__proto__", proto);
    global->set_property("Object", object_prototype);
    global->set_property("String", om.new_object());
    global->set_property("Number", om.new_object());
    global->set_property("Boolean", om.new_object());
    object_prototype->register_native_method(om, "toString", [&](object::Value* value, std::vector<object::Value*>) {
        return om.new_string(value->to_string());
    });

    global->set_property("undefined", om.new_undefined());

    // built in functions
    global->register_native_method(om, "parseInt", [&](object::Value*, std::vector<object::Value*> args) {
        auto num_arg = args[0];

        std::string input;
        if (num_arg->type != object::Value::Type::String) {
            input = num_arg->to_string();
        } else {
            input = num_arg->string();
        }

        auto radix = 10;
        if (args.size() > 1) {
            radix = args[1]->number();
        }
        auto result = std::stoi(input, nullptr, radix);
        return om.new_number(result);
    });

    global->register_native_method(om, "parseFloat", [&](object::Value*, std::vector<object::Value*> args) {
        auto num_arg = args[0];
        std::string input;
        if (num_arg->type != object::Value::Type::String) {
            input = num_arg->to_string();
        } else {
            input = num_arg->string();
        }

        return om.new_number(std::stof(input));
    });

    // console
    auto console = om.new_object();
    om.global_object()->set_property("console", console);

    console->register_native_method(om, "log", [&](object::Value*, std::vector<object::Value*> args) {
        std::string out;

        for (auto arg: args) {
            out += arg->to_json().dump(4) + " ";
        }

        std::cout << out << "\n";

        return om.new_undefined();
    });

    // Math
    auto Math = om.new_object();
    om.global_object()->set_property("Math", Math);
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
    auto array_constructor_handler = [&](object::Value* context, std::vector<object::Value*> args) {
        if (args.size() == 0) {
            return om.new_array();
        }

        auto length = args[0]->number();
        return om.new_array(length);
    };
    auto Array = global->register_native_method(om, "Array", array_constructor_handler);

    Array->register_native_method(om, "fill", [&](object::Value* context, std::vector<object::Value*> args) {
        auto value = om.new_undefined();
        if (args.size() > 0) {
            value = args[0];
        }

        auto array = context->array();
        for (auto &el: array->elements) {
            el = value;
        }

        return context;
    });

    Array->register_native_method(om, "from", [&](object::Value* context, std::vector<object::Value*> args) {
        if (args.size() == 0 || args[0]->type != object::Value::Type::Array) {
            throw_error("TypeError", args[0]->to_string() + " is not iterable");
        }

        std::optional<object::Value*> map_func;
        if (args.size() > 1) {
            map_func = args[1];
        }

        auto old_array = args[0]->array();
        auto new_array = om.new_array();

        for (auto &el: old_array->elements) {
            if (map_func.has_value()) {
                auto mapped = call_function(context, map_func.value(), {el});
                new_array->array()->elements.push_back(mapped);
            } else {
                new_array->array()->elements.push_back(el);
            }
        }

        return new_array;
    });

    Array->register_native_method(om, "push", [&](object::Value* context, std::vector<object::Value*> args) {
        auto array = context->array();

        for (auto arg: args) {
            array->elements.push_back(arg);
        }

        auto length = context->get_property(om, "length");
        assert(length.has_value());
        return length.value();
    });

    Array->register_native_method(om, "pop", [&](object::Value* context, std::vector<object::Value*> args) {
        auto array = context->array();

        if (array->elements.size() == 0) {
            return om.new_undefined();
        }

        auto value = array->elements[array->elements.size() - 1];
        array->elements.pop_back();
        return value;
    });

    Array->register_native_method(om, "forEach", [&](object::Value* context, std::vector<object::Value*> args) {
        auto caller_obj = context->array();
        auto callback = args[0];

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{caller_obj->elements[i], om.new_number(i)};
            call_function(context, callback, args);
        }

        return om.new_undefined();
    });

    Array->register_native_method(om, "map", [&](object::Value* context, std::vector<object::Value*> args) {
        auto caller_obj = context->array();
        auto callback = args[0];

        auto result_value = om.new_array();
        auto result = result_value->array();

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{caller_obj->elements[i], om.new_number(i)};
            auto v = call_function(context, callback, args);
            result->elements.push_back(v);
        }

        return result_value;
    });

    Array->register_native_method(om, "filter", [&](object::Value* context, std::vector<object::Value*> args) {
        auto caller_obj = context->array();
        auto callback = args[0];

        auto result_value = om.new_array();
        auto result = result_value->array();

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{caller_obj->elements[i], om.new_number(i)};
            auto v = call_function(context, callback, args);
            if (v->is_truthy()) {
                result->elements.push_back(v);
            }
        }

        return result_value;
    });

    Array->register_native_method(om, "reduce", [&](object::Value* context, std::vector<object::Value*> args) {
        auto caller_obj = context->array();
        auto callback = args[0];
        auto initial_value = args[1];

        auto prev = initial_value;

        for (auto i = 0; i < caller_obj->elements.size(); i++) {
            auto args = std::vector<object::Value*>{prev, caller_obj->elements[i], om.new_number(i)};
            prev = call_function(context, callback, args);
        }

        return prev;
    });

    // Error
    auto error_constructor_handler = [&](object::Value* context, std::vector<object::Value*> args) {
        auto message = args.size() > 0 ? args[0] : om.new_undefined();
        context->set_property("message", message);
        context->set_property("name", om.new_string("Error"));
        return context;
    };

    auto error_constructor = global->register_native_method(om, "Error", error_constructor_handler);
    auto error_constructor_prototype = error_constructor->get_property(om, "prototype");
    assert(error_constructor_prototype.has_value());

    auto error_constructor_to_string_handler = [&](object::Value* context, std::vector<object::Value*>) {
        auto name = context->get_property(om, "name");
        assert(name.has_value());
        auto message = context->get_property(om, "message");
        assert(message.has_value());
        if (message.value()->is_undefined()) {
            return om.new_string(name.value()->string() + ": undefined\n");
        }

        return om.new_string(name.value()->string() + ": " + message.value()->string());
    };
    error_constructor_prototype.value()->register_native_method(om, "toString", error_constructor_to_string_handler);


    std::vector<std::string> builtin_error_names{"ReferenceError", "TypeError"};

    for (auto name: builtin_error_names) {
        auto handler = [&](object::Value* context, std::vector<object::Value*> args) {
            context->set_property("message", args[0]);
            context->set_property("name", om.new_string(name));
            return context;
        };

        auto reference_error_constructor = global->register_native_method(om, name, handler);
        reference_error_constructor->set_property("prototype", error_constructor_prototype.value());
    }
}

Interpreter::Interpreter() {
    create_builtin_objects();
}

}
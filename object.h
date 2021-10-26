#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "ast.h"

namespace interpreter::object {

enum class ObjectType {
    Undefined,
    Object,
    Function,
    Number,
    String,
    Boolean,
    Array
};

struct ObjectManager;

struct Value {
    using native_function_handler = std::function<Value*(Value*, std::vector<Value*>)>;

    struct Function {
        std::vector<std::string> parameters;
        std::shared_ptr<ast::Statement> body;
        bool is_builtin;
        std::function<Value*(Value*, std::vector<Value*>)> builtin_func;
    };
    struct Array {
        std::vector<Value*> elements;
    };

    enum class Type {
        Object,
        Array,
        Function,
        Number,
        String,
        Boolean,
        Undefined
    };

    std::unordered_map<std::string, Value*> properties;
    std::optional<std::string> prototype;
    Type type;
    std::variant<double, std::string, bool, std::vector<Value*>, std::unordered_map<std::string, Value*>, Function, Array> value;

    Function* function() {
        assert(type == Type::Function);
        return &std::get<Function>(value);
    }

    Array* array() {
        assert(type == Type::Array);
        return &std::get<Array>(value);
    }

    double number() {
        assert(type == Type::Number);
        return std::get<double>(value);
    }

    std::string string() {
        assert(type == Type::String);
        return std::get<std::string>(value);
    }

    std::string to_string() {
        switch (type) {
            case Type::Object: {
                nlohmann::json j;

                for (auto p: properties) {
                    j[p.first] = p.second->to_string();
                }

                if (j.empty()) {
                    return "{}";
                }

                return j.dump(4);
            }
            case Type::Function:
                return "Function";
            case Type::Array: {
                nlohmann::json j;

                std::vector<std::string> element_strings;

                auto array = std::get<Array>(value);
                for (auto e: array.elements) {
                    element_strings.push_back(e->to_string());
                }

                j = element_strings;

                if (j.empty()) {
                    return "[]";
                }

                return j.dump(4);
            }
            case Type::Number:
                return std::to_string(std::get<double>(value));
            case Type::String:
                return "\"" + std::get<std::string>(value) + "\"";
            case Type::Boolean:
                return std::get<bool>(value) ? "true" : "False";
            case Type::Undefined:
                return "undefined";
        }
    }

    bool is_truthy() {
        switch (type) {
            case Type::Object:
            case Type::Function:
            case Type::Array:
                return true;
            case Type::Number:
                return std::get<double>(value) != 0;
            case Type::String:
                return std::get<std::string>(value) != "";
            case Type::Boolean:
                return std::get<bool>(value);
            case Type::Undefined:
                return false;
        }
    }

    bool is_undefined() {
        return type == Type::Undefined;
    }

    Value* get_property(ObjectManager &object_manager, std::string name);
    Value* get_property(ObjectManager &object_manager, int index);
    Value* set_property(std::string name, Value* value);
    Value* set_property(int index, Value* value);

    void register_native_method(ObjectManager &object_manager, std::string name, native_function_handler handler);
};


class ValueFactory {
    ValueFactory();
public:
    static Value* number(Value* value, double v) {
        value->type = Value::Type::Number;
        value->value = v;
        value->prototype = "Number";
        return value;
    }

    static Value* string(Value* value, std::string v) {
        value->type = Value::Type::String;
        value->value = v;
        value->prototype = "String";
        return value;
    }

    static Value* boolean(Value* value, bool v) {
        value->type = Value::Type::Boolean;
        value->value = v;
        value->prototype = "Boolean";
        return value;
    }

    static Value* array(Value* value) {
        std::vector<Value*> v;
        return array(value, v);
    }

    static Value* array(Value* value, std::vector<Value*> v) {
        value->type = Value::Type::Array;
        value->value = Value::Array{v};
        value->prototype = "Array";
        return value;
    }

    static Value* object(Value* value) {
        std::unordered_map<std::string, Value*> v;
        return object(value, v);
    }

    static Value* object(Value* value, std::unordered_map<std::string, Value*> v) {
        value->type = Value::Type::Object;
        value->value = v;
        value->prototype = "Object";
        return value;
    }

    static Value* function(Value* value) {
        value->type = Value::Type::Function;
        value->value = Value::Function{};
        value->prototype = "Object";
        return value;
    }

    static Value* undefined(Value* value) {
        value->type = Value::Type::Undefined;
        value->prototype = "Object";
        return value;
    }
};


class ObjectManager {
    struct Scope {
        std::unordered_set<Value*> values_in_scope;
        std::unordered_map<std::string, Value*> variables;

        Value* get_variable(std::string name) {
            if (auto entry = variables.find(name); entry != variables.end()) {
                return entry->second;
            }

            return nullptr;
        }

        Value* set_variable(std::string name, Value* value) {
            variables[name] = value;
            return value;
        }
    };

    struct Cell {
        Value* value;
        bool is_referenced;
    };

    std::vector<Scope> scopes;
    int current_scope_index = 0;

    std::unordered_map<Value*, Cell> objects;

    const int gc_threshold = 25000;
    int gc_amount = 0;
    int objects_collected = 0;

    void collect_garbage();

    template<typename T, typename ... Args>
    T* allocate(Args &&... args);

    bool is_value_still_in_scope(Value* v) {
        auto i = current_scope_index;

        while (i >= 0) {
            auto scope = scopes[i];
            auto value = scope.values_in_scope.find(v);
            if (value != scope.values_in_scope.end()) {
                return true;
            }

            i--;
        }

        return false;
    }

public:
    ObjectManager() {
        // push top level scope
        scopes.push_back(Scope{});
    }

    Value* new_object() {
        return ValueFactory::object(allocate<Value>());
    }
    Value* new_function() {
        return ValueFactory::function(allocate<Value>());
    }
    Value* new_array() {
        return ValueFactory::array(allocate<Value>());
    }
    Value* new_number(double value) {
        return ValueFactory::number(allocate<Value>(), value);
    }
    Value* new_string(std::string value) {
        return ValueFactory::string(allocate<Value>(), value);
    }
    Value* new_boolean(bool value) {
        return ValueFactory::boolean(allocate<Value>(), value);
    }
    Value* new_undefined() {
        return ValueFactory::undefined(allocate<Value>());
    }

    void push_scope();
    void pop_scope();
    Scope* current_scope();
    Scope* global_scope();
    Value* get_variable(std::string name);
    Value* set_variable(std::string name, Value* value);
    Value* declare_variable(std::string name, Value* value);
};

}
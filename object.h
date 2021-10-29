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
        std::optional<std::string> name;
        std::vector<std::string> parameters;
        std::shared_ptr<ast::ASTNode> body;
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

    nlohmann::json to_json() {
        switch (type) {
            case Type::Object: {
                nlohmann::json j;

                for (auto p: properties) {
                    if (p.first == "__proto__") {
                        continue;
                    }
                    j[p.first] = p.second->to_json();
                }

                if (j.empty()) {
                    return "{}";
                }

                return j;
            }
            case Type::Function:
                return function()->is_builtin ? "Native Function" : "Function";
            case Type::Array: {
                nlohmann::json j;

                std::vector<nlohmann::json> element_strings;

                auto array = std::get<Array>(value);
                for (auto e: array.elements) {
                    element_strings.push_back(e->to_json());
                }

                j = element_strings;

                return j;
            }
            case Type::Number:
                return std::get<double>(value);
            case Type::String:
                return std::get<std::string>(value);
            case Type::Boolean:
                return std::get<bool>(value);
            case Type::Undefined:
                return "undefined";
        }
    }


    std::string to_string() {
        switch (type) {
            case Type::Object: {
                auto out = to_json().dump(4);
                auto prototype_entry = properties.find("__proto__");
                if (prototype_entry == properties.end()) {
                    return out;
                }

                out = "[object ";

                auto prototype = prototype_entry->second;
                auto constructor = prototype->properties.find("constructor");
                if (constructor != prototype->properties.end()) {
                    assert(constructor->second->type == Type::Function);
                    auto name = constructor->second->function()->name;
                    if (name.has_value()) {
                        out += name.value();
                        out += "]";
                        return out;
                    }
                }

                out += "Object]";

                return out;
            }
            case Type::Function:
            case Type::Array:
            case Type::Number:
            case Type::String:
            case Type::Boolean:
            case Type::Undefined:
                return to_json().dump(4);
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

    std::optional<Value*> get_property(ObjectManager &object_manager, std::string name);
    std::optional<Value*> get_property(ObjectManager &object_manager, int index);
    Value* set_property(std::string name, Value* value);
    Value* set_property(int index, Value* value);

    Value* register_native_method(ObjectManager &object_manager, std::string name, native_function_handler handler);
};


class ValueFactory {
    ValueFactory();
public:
    static Value* number(ObjectManager &om, Value* value, double v);
    static Value* string(ObjectManager &om, Value* value, std::string v);
    static Value* boolean(ObjectManager &om, Value* value, bool v);
    static Value* array(ObjectManager &om, Value* value, std::optional<int> length);
    static Value* array(ObjectManager &om, Value* value, std::vector<Value*> v, std::optional<int> length);
    static Value* object(ObjectManager &om, Value* value);
    static Value* object(ObjectManager &om, Value* value, std::unordered_map<std::string, Value*> v);
    static Value* function(ObjectManager &om, Value* value, std::optional<std::string> name);
    static Value* undefined(ObjectManager &om, Value* value);
};


class ObjectManager {
    struct Scope {
        ObjectManager &om;
        Value* context;
        bool is_global;

        Scope(ObjectManager &om, Value* context, bool is_global = false)
                : om(om), context(context), is_global(is_global) {}

        std::unordered_set<Value*> values_in_scope;
        std::unordered_map<std::string, Value*> variables;

        std::optional<Value*> get_variable(std::string name) {
            if (is_global) {
                return context->get_property(om, name);
            }

            if (auto entry = variables.find(name); entry != variables.end()) {
                return entry->second;
            }

            return {};
        }

        Value* set_variable(std::string name, Value* value) {
            if (is_global) {
                return context->set_property(name, value);
            }

            variables[name] = value;
            return value;
        }

        Value* this_context() {
            return context;
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

    Value* global;

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
        global = new_object();
        // push top level scope
        scopes.push_back(Scope{*this, global, true});
    }

    Value* new_object() {
        return ValueFactory::object(*this, allocate<Value>());
    }
    Value* new_function() {
        return ValueFactory::function(*this, allocate<Value>(), {});
    }
    Value* new_function(std::optional<std::string> name) {
        return ValueFactory::function(*this, allocate<Value>(), name);
    }
    Value* new_array() {
        return ValueFactory::array(*this, allocate<Value>(), {});
    }
    Value* new_array(int length) {
        return ValueFactory::array(*this, allocate<Value>(), length);
    }
    Value* new_number(double value) {
        return ValueFactory::number(*this, allocate<Value>(), value);
    }
    Value* new_string(std::string value) {
        return ValueFactory::string(*this, allocate<Value>(), value);
    }
    Value* new_boolean(bool value) {
        return ValueFactory::boolean(*this, allocate<Value>(), value);
    }
    Value* new_undefined() {
        return ValueFactory::undefined(*this, allocate<Value>());
    }

    void push_scope(Value* context);
    void pop_scope();
    Scope* current_scope();
    Scope* global_scope();
    Value* global_object();
    std::optional<object::Value*> get_variable(std::string name);
    Value* set_variable(std::string name, Value* value);
    Value* declare_variable(std::string name, Value* value);
};

}
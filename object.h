#pragma once

#include <string>
#include <unordered_map>

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

struct Object {
    Object(ObjectType object_type) : object_type(object_type) {}
    ObjectType object_type = ObjectType::Object;
    std::unordered_map<std::string, Object*> properties;

    ObjectType type() { return object_type; }
    virtual bool is_truthy() { return true; }

    virtual std::string to_string() {
        nlohmann::json j;

        for (auto p: properties) {
            j[p.first] = p.second->to_string();
        }

        if (j.empty()) {
            return "{}";
        }

        return j.dump(4);
    }

    virtual Object* get_property(std::string name) {
        if (auto entry = properties.find(name); entry != properties.end()) {
            return entry->second;
        }

        return nullptr;
    }

    virtual Object* get_property(int index) {
        auto name = std::to_string(index);
        return get_property(name);
    }

    virtual Object* set_property(std::string name, Object* value) {
        properties[name] = value;
        return value;
    }

    virtual Object* set_property(int index, Object* value) {
        auto name = std::to_string(index);
        return set_property(name, value);
    }
};

struct Function : public Object {
    Function() : Object(ObjectType::Function) {}
    bool is_builtin;
    std::function<Object*(std::vector<Object*>)> builtin_func;
    std::vector<std::string> parameters;
    std::shared_ptr<ast::Statement> body;
    bool is_truthy() override { return true; }
    std::string to_string() override { return "Function"; }
};

struct Undefined : public Object {
    Undefined() : Object(ObjectType::Undefined) {}
    bool is_truthy() override { return false; }
    std::string to_string() override { return "undefined"; }
};

struct Number : public Object {
    Number(double value) : Object(ObjectType::Number), value(value) {};
    double value;
    bool is_truthy() override { return value != 0; }
    std::string to_string() override { return std::to_string(value); }
};

struct String : public Object {
    String(std::string value) : Object(ObjectType::String), value(value) {};
    std::string value;
    bool is_truthy() override { return value != ""; }
    std::string to_string() override { return "\"" + value + "\""; }
};

struct Boolean : public Object {
    Boolean(bool value) : Object(ObjectType::Boolean), value(value) {};
    bool value;
    bool is_truthy() override { return value; }
    std::string to_string() override { return value ? "true" : "false"; }
};

struct Array : public Object {
    Array() : Object(ObjectType::Array) {}
    std::vector<Object*> elements;
    bool is_truthy() override { return true; }
    std::string to_string() override {
        nlohmann::json j;

        std::vector<std::string> element_strings;

        for (auto e: elements) {
            element_strings.push_back(e->to_string());
        }

        j = element_strings;

        if (j.empty()) {
            return "[]";
        }

        return j.dump(4);
    }

    Object* get_property(std::string name) override {
        // TODO: this number won't always be accurate
        if (name == "length") {
            // TODO: this needs to go through object_manager for when we implement garbage collection
            return new Number(elements.size());
        }

        return Object::get_property(name);
    }

    Object* get_property(int index) override {
        if ((index + 1) > elements.size()) {
            return nullptr;
        }

        return elements.at(index);
    }

    Object* set_property(int index, Object* value) override {
        if (index > elements.size()) {
            elements.resize(index + 1);
        }

        elements[index] = value;
        return value;
    }
};


class ObjectManager {
public:
    Object* new_object();
    Function* new_function();
    Array* new_array();
    Number* new_number(double value);
    String* new_string(std::string value);
    Boolean* new_boolean(bool value);
    Undefined* new_undefined();
    bool is_undefined(Object* value);
};

}
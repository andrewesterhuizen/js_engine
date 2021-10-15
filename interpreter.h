#pragma once

#include <string>
#include <unordered_map>

#include "ast.h"

namespace interpreter {

struct Object {
    std::unordered_map<std::string, Object*> properties;

    virtual bool is_truthy() {
        return true;
    }

    virtual std::string to_string() {
        return "Object";
    }

    Object* get_propery(std::string name) {
        if (auto entry = properties.find(name); entry != properties.end()) {
            return entry->second;
        }

        return nullptr;
    }
};

struct Function : public Object {
    bool is_builtin;
    std::function<Object*(std::vector<Object*>)> func;

    bool is_truthy() override {
        return true;
    }

    std::string to_string() override {
        return "Function";
    }
};

struct Undefined : public Object {
    bool is_truthy() override {
        return false;
    }

    std::string to_string() override {
        return "undefined";
    }
};

struct Number : public Object {
    Number(double value) : value(value) {};
    double value;

    bool is_truthy() override {
        return value != 0;
    }

    std::string to_string() override {
        return std::to_string(value);
    }
};

struct String : public Object {
    String(std::string value) : value(value) {};
    std::string value;

    bool is_truthy() override {
        return value != "";
    }

    std::string to_string() override {
        return value;
    }
};

struct Boolean : public Object {
    Boolean(bool value) : value(value) {};
    bool value;

    bool is_truthy() override {
        return value;
    }

    std::string to_string() override {
        return value ? "true" : "false";
    }
};

class ObjectManager {
    Undefined undefined;

public:
    Object* new_object();
    Function* new_function();
    Number* new_number(double value);
    String* new_string(std::string value);
    Boolean* new_boolean(bool value);
    Undefined* new_undefined();
    bool is_undefined(Object* value);
};

class Interpreter {
    ObjectManager object_manager;
    // TODO: actual function scopes
    std::unordered_map<std::string, Object*> variables;

    Object* lookup_variable(std::string name);

public:
    Interpreter();
    Object* execute(std::shared_ptr<ast::Statement> statement);
    Object* execute(std::shared_ptr<ast::Expression> expression);
    void run(ast::Program &program);
};

}
#pragma once

#include <string>
#include <unordered_map>

#include "ast.h"
#include "object.h"

namespace interpreter {

struct Error {
    std::string type;
    std::string message;
};

struct Scope {
    std::unordered_map<std::string, object::Object*> variables;

    object::Object* get_variable(std::string name) {
        if (auto entry = variables.find(name); entry != variables.end()) {
            return entry->second;
        }

        return nullptr;
    }

    object::Object* set_variable(std::string name, object::Object* value) {
        variables[name] = value;
        return value;
    }
};

class Interpreter {
    object::ObjectManager object_manager;
    std::vector<Scope> scopes;
    int current_scope_index = 0;

    void push_scope();
    void pop_scope();
    Scope* current_scope();

    object::Object* get_variable(std::string name);
    object::Object* set_variable(std::string name, object::Object* value);

    object::Object* execute(std::shared_ptr<ast::Statement> statement);
    object::Object* execute(std::shared_ptr<ast::Expression> expression);
    void throw_error(std::string type, std::string message);
public:
    Interpreter();
    void run(ast::Program &program);
};

}
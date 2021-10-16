#pragma once

#include <string>
#include <unordered_map>

#include "ast.h"
#include "object.h"

namespace interpreter {

class Interpreter {
    object::ObjectManager object_manager;
    // TODO: actual function scopes
    std::unordered_map<std::string, object::Object*> variables;

    object::Object* get_variable(std::string name);
    object::Object* set_variable(std::string name, object::Object* value);

public:
    Interpreter();
    object::Object* execute(std::shared_ptr<ast::Statement> statement);
    object::Object* execute(std::shared_ptr<ast::Expression> expression);
    void run(ast::Program &program);
};

}
#pragma once

#include <string>
#include <unordered_map>

#include "ast.h"
#include "object.h"

namespace interpreter {

class Interpreter {
    struct Return {
        object::Value* value;
    };

    object::ObjectManager om;

    object::Value* get_variable(std::string name);
    object::Value* set_variable(std::string name, object::Value* value);
    object::Value* declare_variable(std::string name, object::Value* value);

    object::Value* call_function(object::Value* caller, object::Value* func_value, std::vector<object::Value*> args);
    object::Value* execute(std::shared_ptr<ast::Statement> statement);
    object::Value* execute(std::shared_ptr<ast::Expression> expression);
    void throw_error(std::string type, std::string message);

    void create_builtin_objects();
public:
    Interpreter();
    void run(ast::Program &program);
};

}
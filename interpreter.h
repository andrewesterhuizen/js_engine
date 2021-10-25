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

class Interpreter {
    object::ObjectManager object_manager;

    object::Object* get_variable(std::string name);
    object::Object* set_variable(std::string name, object::Object* value);
    object::Object* declare_variable(std::string name, object::Object* value);

    object::Object* execute(std::shared_ptr<ast::Statement> statement);
    object::Object* execute(std::shared_ptr<ast::Expression> expression);
    void throw_error(std::string type, std::string message);
public:
    Interpreter();
    void run(ast::Program &program);
};

}
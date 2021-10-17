#include <iostream>
#include <string>

#include "json.hpp"
#include "ast.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

int main() {
    std::string source = R"(
        var obj = {
            x: "hello from property set in declaration",
            y: 1.234
        };
        obj.z = "hello from property assignment";

        console.log(obj);
        console.log(obj.x);
        console.log(obj.y);
        console.log(obj.z);
    )";

    // get tokens
    lexer::Lexer l;
    auto tokens = l.get_tokens(source);

    // print tokens
    nlohmann::json j;
    std::vector<nlohmann::json> out;
    for (auto t: tokens) {
        out.push_back(t.to_json());
    }

    j = out;
    std::cout << j.dump(4) << "\n";

    // parse tokens
    parser::Parser p;
    auto ast = p.parse(tokens);
    std::cout << ast.to_json().dump(4) << "\n";

    // execute ast
    interpreter::Interpreter i;
    i.run(ast);

    return 0;
}

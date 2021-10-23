#include <iostream>
#include <string>

#include "json.hpp"
#include "ast.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

int main() {
    std::string source = R"(
        var x = 1;
        x++;
        console.log(x);
        x--;
        console.log(x);
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

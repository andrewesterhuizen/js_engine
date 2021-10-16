#include <iostream>
#include <string>

#include "json.hpp"
#include "ast.h"
#include "interpreter.h"
#include "lexer.h"

int main() {
    std::string source = R"(
        var message = "Hello";
        console.log(message);
    )";

    lexer::Lexer l;
    auto tokens = l.get_tokens(source);

    nlohmann::json j;
    std::vector<nlohmann::json> out;
    for (auto t: tokens) {
        out.push_back(t.to_json());
    }

    j = out;

    std::cout << j.dump(4) << "\n";

    return 0;
}

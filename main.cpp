#include <iostream>
#include <string>
#include <set>
#include <fstream>
#include <sstream>

#include "json.hpp"
#include "ast.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

std::vector<std::string> get_files(std::string files_arg) {
    std::string arg_name = "--files=";
    std::vector<std::string> files;

    auto files_args_list = files_arg.substr(arg_name.length(), files_arg.size());

    std::string out;
    std::stringstream ss(files_args_list);

    while (getline(ss, out, ',')) {
        files.push_back(out);
    }

    return files;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "no file specified\n";
        return 1;
    }

    auto files_arg = argv[1];
    std::set<std::string> args(argv + 1, argv + argc);

    for (auto arg: args) {
        std::cout << arg << "\n";
    }

    auto output_tokens = args.find("--output-tokens") != args.end();
    auto output_ast = args.find("--output-ast") != args.end();

    auto files = get_files(files_arg);

    std::stringstream source_buffer;

    for (auto f: files) {
        std::ifstream file(f);
        if (!file.is_open()) {
            std::cerr << "unable to open file " << f << "\n";
            return 1;
        }

        source_buffer << file.rdbuf();
    }

    auto source = source_buffer.str();

    // get tokens
    lexer::Lexer l;
    auto tokens = l.get_tokens(source);

    if (output_tokens) {
        // print tokens
        nlohmann::json j;
        std::vector<nlohmann::json> out;
        for (auto t: tokens) {
            out.push_back(t.to_json());
        }

        j = out;
        std::cout << j.dump(4) << "\n";

        return 0;
    }

    // parse tokens
    parser::Parser p;
    auto ast = p.parse(tokens);

    if (output_ast) {
        std::cout << ast.to_json().dump(4) << "\n";
        return 0;
    }

    // execute ast
    interpreter::Interpreter i;
    i.run(ast);

    return 0;
}

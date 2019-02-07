#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "interpreter.hpp"
#include "frontend/parser/ast/ast.hpp"
#include "frontend/parser/ast/ast_printer.hpp"
#include "frontend/parser/ast/ast_common.hpp"
#include "frontend/parser/ast/ast_decl.hpp"
#include "frontend/parser/ast/ast_expr.hpp"
#include "frontend/parser/ast/ast_typespec.hpp"

#include "utils/directory.hpp"
#include "vm.hpp"


int main(int argc, const char** argv) {
    if(argc < 2)
        return 1;

	auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::string> args;
    for(int i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    mist::Interpreter interp(args);

    interp.compile_root();

	auto end = std::chrono::high_resolution_clock::now();

	auto diff = end - start;
	std::cout << "Time: " << static_cast<std::chrono::duration<double>>(diff).count() << std::endl;
}

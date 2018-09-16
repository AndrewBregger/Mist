#include <iostream>
#include <vector>
#include <string>

#include "interpreter.hpp"

int main(int argc, const char** argv) {
    if(argc < 2)
        return 1;

    std::vector<std::string> args;
    for(int i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    mist::Interpreter interp(args);    

    interp.compile_root();
}

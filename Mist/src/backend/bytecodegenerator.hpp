//
// Created by Andrew Bregger on 2019-02-18.
//

#pragma once

#include "interpreter.hpp"

namespace ast {
    class Module;
}

namespace mist {
    class BytecodeGenerator {
    public:
        BytecodeGenerator(Interpreter* interp);

        ~BytecodeGenerator();

        void run(ast::Module *module);
    private:
        Interpreter* interp;
    };
}


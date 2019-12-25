//
// Created by Andrew Bregger on 2019-02-18.
//

#include "bytecodegenerator.hpp"

#include <frontend/parser/ast/ast.hpp>
#include <frontend/parser/ast/ast_expr.hpp>
#include <frontend/parser/ast/ast_decl.hpp>
#include <frontend/parser/ast/ast_typespec.hpp>

#include "vm.hpp"

namespace mist {
    BytecodeGenerator::BytecodeGenerator(Interpreter* interp) : interp(interp) {}

    BytecodeGenerator::~BytecodeGenerator() = default;

    void BytecodeGenerator::run(ast::Module *module) {

    }
}
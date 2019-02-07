//
// Created by Andrew Bregger on 2019-01-30.
//

#pragma once

#include "interpreter.hpp"
#include "frontend/parser/ast/ast.hpp"
#include "frontend/parser/ast/ast_expr.hpp"
#include "frontend/parser/ast/ast_decl.hpp"
#include "frontend/parser/ast/ast_typespec.hpp"

namespace mist {
    class Desurgur {
    public:
        Desurgur(Interpreter* interp);

        void process_decl(ast::Decl* decl);

        void process_structures(ast::StructDecl* decl);


        std::vector<ast::Pattern*> expand_field_pattern(ast::Pattern* pat, ast::Expr* init);

    private:

        Interpreter* interp{nullptr};
    };
}


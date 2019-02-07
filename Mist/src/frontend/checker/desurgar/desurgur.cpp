//
// Created by Andrew Bregger on 2019-01-30.
//

#include "desurgur.hpp"

namespace mist {
    Desurgur::Desurgur(Interpreter *interp) : interp(interp) {
    }

    void Desurgur::process_decl(ast::Decl *decl) {
        switch(decl->kind()) {
            case ast::Struct:
                process_structures(CAST_PTR(ast::StructDecl, decl));
        }
    }

    void Desurgur::process_structures(ast::StructDecl *decl) {
        auto members = decl->fields;
        for(auto field : members) {

        }
    }

    std::vector<ast::Pattern *> Desurgur::expand_field_pattern(ast::Pattern *pat, ast::Expr* init) {
        std::vector<ast::Pattern*> pats;

        switch(pat->kind()) {
            case ast::IdentPatKind:
            case ast::UnderscorePatKind:
            case ast::TuplePatKind:
            case ast::StructurePatKind:
            case ast::VariantPatKind:
            case ast::IntegerLiteralPatKind:
            case ast::FloatLiteralPatKind:
            case ast::StringLiteralPatKind:
            case ast::CharLiteralPatKind:
            case ast::BooleanPatKind:
            case ast::RangePatKind:
            case ast::ListPatKind:
            case ast::OptionalPatKind:
            case ast::FunctionalPatKind:
                break;
        }

        return pats;
    }
}

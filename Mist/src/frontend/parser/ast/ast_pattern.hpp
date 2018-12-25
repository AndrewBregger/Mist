//
// Created by Andrew Bregger on 2018-12-24.
//

#pragma once

#include "ast_common.hpp"

namespace ast {

    struct Expr;

    enum PatternKind {
        IdentPatKind,
        TuplePatKind,
        StructurePatKind,
        VariantPatKind,
        IntegerLiteralPatKind,
        FloatLiteralPatKind,
        StringLiteralPatKind,
        CharLiteralPatKind,
        BooleanPatKind,
        RangePatKind
    };

    struct Pattern {
        PatternKind  k;
        mist::Pos p;


        Pattern(PatternKind k, mist::Pos p);

        inline mist::Pos pos() { return p; }

        inline PatternKind kind() { return k; }
    };

    struct IdentPat : public Pattern {
        ast::Ident* name;

        IdentPat(ast::Ident* name, mist::Pos pos);
    };

    struct TuplePat : public Pattern {
        std::vector<ast::Pattern*> elements;

        TuplePat(const std::vector<ast::Pattern*>& elements, mist::Pos pos);
    };

    struct StructPat : public Pattern {
        ast::Expr* name;
        std::vector<ast::Pattern*> members;

        StructPat(ast::Expr* name, const std::vector<ast::Pattern*>& members, mist::Pos pos);
    };

    struct VariantPat : public Pattern {
        ast::Expr* name;
        std::vector<ast::Pattern*> members;

        VariantPat(ast::Expr* name, const std::vector<ast::Pattern*>& members, mist::Pos pos);
    };

    struct IntegerPat : public Pattern {
        i64 value;

        IntegerPat(i64 value, mist::Pos pos);
    };

    struct FloatPat : public Pattern {
        float value;

        FloatPat(float value, mist::Pos pos);
    };

    struct StringPat : public Pattern {
        mist::String* value;

        StringPat(mist::String* value, mist::Pos pos);
    };

    struct CharacterPat : public Pattern {
        char value;

        CharacterPat(char value, mist::Pos pos);
    };

    struct BooleanPat : public Pattern {
        bool value;

        BooleanPat(bool value, mist::Pos pos);
    };

    struct RangePat : public Pattern {
        Expr* low,* high;

        RangePat(Expr* low, Expr* high, mist::Pos pos);
    };

}

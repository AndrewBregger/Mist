//
// Created by Andrew Bregger on 2018-12-24.
//

#pragma once

#include "ast_common.hpp"

namespace ast {

    struct Expr;

    /// @note: I am considering adding a pattern that can handle abitrary boolean expressions.
    /// I am not sure about the syntax. Maybe the solution is to use currying to solve this.

    enum PatternKind {
        IdentPatKind,
        UnderscorePatKind,
        TuplePatKind,
        StructurePatKind,
        VariantPatKind,
        IntegerLiteralPatKind,
        FloatLiteralPatKind,
        StringLiteralPatKind,
        CharLiteralPatKind,
        BooleanPatKind,
        RangePatKind,
        ListPatKind,
        OptionalPatKind,
        FunctionalPatKind
    };

    struct Pattern {
        PatternKind  k;
        mist::Pos p;


        Pattern(PatternKind k, mist::Pos p);

        inline mist::Pos pos() { return p; }

        inline PatternKind kind() { return k; }

        const std::string& name();
    };

    struct IdentPat : public Pattern {
        ast::Ident* name;

        IdentPat(ast::Ident* name, mist::Pos pos);
    };

    struct UnderscorePat : public Pattern {
        UnderscorePat(mist::Pos pos);
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
        struct mist::String* value;

        StringPat(struct mist::String* value, mist::Pos pos);
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
        bool inclusive;

        RangePat(Expr *low, Expr *high, bool inclusive, mist::Pos pos);
    };

    struct ListPat : public Pattern {
        std::vector<Pattern*> patterns;

        ListPat(const std::vector<Pattern*>& paterns, mist::Pos pos);
    };

    // implement the others
}

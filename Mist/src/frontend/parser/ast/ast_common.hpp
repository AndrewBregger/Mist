#pragma once

#include "common.hpp"
#include <vector>
//#include "interpreter.hpp"

namespace mist {
    struct Pos {
        u32 line{0};
        u32 column{0};
        u32 span{0};
        ::u64 fileId{0};

        Pos();

        Pos(u32 line, u32 column, u32 span, ::u64 fileId = 0);

        Pos operator+ (const Pos& p);
    };
	
	struct String;
}

namespace ast {
    enum Visibility {
        Public,
        Private
    };

    enum Mutablity {
        Mutable,
        Immutable
    };

    struct Ident {
        mist::String* value;
        mist::Pos pos;

        Ident(mist::String* value, const mist::Pos& pos);
    };

    struct TypeSpec;

    struct WhereElement {
        Ident* parameter;
        TypeSpec* type;
        mist::Pos pos;

        WhereElement(Ident* parameter, TypeSpec* type, mist::Pos pos);
    };

    struct WhereClause {
        std::vector<WhereElement*> elements;
        mist::Pos& pos;

        WhereClause(const std::vector<WhereElement*>& elems, mist::Pos pos);
    };

    struct Path {
        // std::vector<PathElement*> fields;
        // Path(const std::vector<PathElement*> fields);
    };
}
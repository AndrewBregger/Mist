#pragma once

#include "common.hpp"
#include "interpreter.hpp"

namespace mist {
    struct Pos {
        u32 line{0};
        u32 column{0};
        u32 span{0};
        ::u64 fileId{0};

        Pos();
        Pos(u32 line, u32 column, u32 span, ::u64 fileId = 0);
    };
}

namespace ast {
    struct Ident {
        mist::String* value;
        mist::Pos pos;

        Ident(mist::String* value, const mist::Pos& pos);
    };
}
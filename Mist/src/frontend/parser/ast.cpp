#include "ast.hpp"

namespace mist {
    Pos::Pos(u32 line, u32 column, u32 span, u64 fileId) : line(line), column(column),
        span(span), fileId(fileId) {}
}

namespace ast {
    Ident::Ident(mist::String* value, const mist::Pos& pos) : value(value), pos(pos) {
    }
}

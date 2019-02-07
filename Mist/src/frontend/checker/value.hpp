//
// Created by Andrew Bregger on 2019-02-03.
//

#pragma once

#include "common.hpp"


// for now this will only evaluate primitive values.

namespace ast {
    struct Expr;
}

namespace mist {
    class Type;


    struct Val {
        Type* type{nullptr};
        ast::Expr* expr{nullptr};
        bool is_constant{false};
        union {
            i8  _I8;
            i16 _I16;
            i32 _I32;
            i64 _I64;

            u8  _U8;
            u16 _U16;
            u32 _U32;
            u64 _U64;

            f32 _F32;
            f64 _F64;
            char _Char;
            bool _Bool;
        };

        Val();
        explicit Val(Type* type);
        explicit Val(i8 val);
        explicit Val(i16 val);
        explicit Val(i32 val);
        explicit Val(i64 val);
        explicit Val(u8 val);
        explicit Val(u16 val);
        explicit Val(u32 val);
        explicit Val(u64 val);
        explicit Val(f32 val);
        explicit Val(f64 val);
        explicit Val(char val);
        explicit Val(bool _bool);

        friend std::ostream& operator<< (std::ostream& out, const Val& val);
    };

}

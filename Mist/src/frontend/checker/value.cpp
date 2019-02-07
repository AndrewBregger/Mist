//
// Created by Andrew Bregger on 2019-02-03.
//

#include <fstream>
#include "value.hpp"
#include "types/type.hpp"

#define Constructor(ctype, vtype, mtype) Val::Val(ctype val) : type(mtype), is_constant(true) { _##vtype = val; }


extern mist::Type* type_u8;
extern mist::Type* type_u16;
extern mist::Type* type_u32;
extern mist::Type* type_u64;
extern mist::Type* type_i8;
extern mist::Type* type_i16;
extern mist::Type* type_i32;
extern mist::Type* type_i64;
extern mist::Type* type_f32;
extern mist::Type* type_f64;
extern mist::Type* type_char;
extern mist::Type* type_bool;

namespace mist {
    Val::Val() = default;
    Val::Val(Type* type) : type(type), is_constant(false) {}

    Constructor(i8 , I8,  type_i8)
    Constructor(i16, I16, type_i16)
    Constructor(i32, I32, type_i32)
    Constructor(i64, I64, type_i64)
    Constructor(u8 , U8,  type_u8)
    Constructor(u16, U16, type_u16)
    Constructor(u32, U32, type_u32)
    Constructor(u64, U64, type_u64)
    Constructor(f32, F32, type_f32)
    Constructor(f64, F64, type_f64)
    Constructor(char, Char, type_char)
    Constructor(bool, Bool, type_bool)

    std::ostream &operator<<(std::ostream &out, const Val &val) {
        if(val.is_constant) {
            if(val.type == type_u8) {
                out << val._U8;
            }
            else if(val.type == type_u16) {
                out << val._U16;
            }
            else if(val.type == type_u32) {
                out << val._U32;
            }
            else if(val.type == type_u64) {
                out << val._U64;
            }
            else if(val.type == type_i8) {
                out << val._I8;
            }
            else if(val.type == type_i16) {
                out << val._I16;
            }
            else if(val.type == type_i32) {
                out << val._I32;
            }
            else if(val.type == type_i64) {
                out << val._I64;
            }
            else if(val.type == type_f32) {
                out << val._F32;
            }
            else if(val.type == type_f64) {
                out << val._F64;
            }
            else if(val.type == type_char) {
                out << val._Char;
            }
            else if(val.type == type_bool) {
                out << (val._Bool ? "true" :  "false");
            }
        }
        else {
            out << "no value";
        }
        return out;
    }
}

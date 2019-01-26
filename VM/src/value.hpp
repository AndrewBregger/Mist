#pragma once

#include "common.hpp"
#include "value.hpp"
#include <fstream>

enum ValueKind {
    Integer,
    Float,
    Boolean,
    Nill,
    Function,
    Obj,
    Sequence, // this is a list of values
};

struct Object;

struct Value {
    ValueKind kind;
    union {
        i64 i;
        f64 f;
        bool b;
        Object* obj;
        struct {
            Value* elems;
            u32 num;
        } seq;
    };

    Value();
    Value(i64 i);
    Value(f64 f);
    Value(bool b);
    Value(Object* o);

    // @note: this value owns this memory
    Value(Value* elems, u32 num);

    Value(const Value& other);

    Value operator= (const Value& other);

    // Value(Value&& other);

    ~Value();

    inline bool is_integer() { return kind == Integer; }
    inline bool is_float() { return kind == Float; }
    inline bool is_object() { return kind == Obj; }
    inline bool is_boolean() { return kind == Boolean; }
    inline bool is_sequence() { return kind == Sequence; }

    friend std::ostream& operator <<(std::ostream& out, const Value& val);
};

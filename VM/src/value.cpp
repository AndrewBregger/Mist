#include "value.hpp"
#include "object.hpp"
#include <cstdlib>

/*
    TODO:
        Refactor the memroy handling of sequence values.
        The current Implemention assumes the value owns the
        memory so it gets copied every time. This gets expensive.
*/

Value::Value() : kind(Nill), i(0) {

}
Value::Value(i64 i) : kind(Integer), i(i) {
}

Value::Value(f64 f) : kind(Float), f(f) {
}

Value::Value(bool b) : kind(Boolean), b(b) {
}

Value::Value(Object* obj) : kind(Obj), obj(obj) {
}

Value::Value(Value* elems, u32 num) : kind(Sequence) {
    seq.elems = (Value*) malloc(num * sizeof(Value));
    // memcpy(seq.elems, elems, num * sizeof(Value));
    for(u32 i = 0; i < num; ++i)
        seq.elems[i] = elems[i];
    seq.num = num;
}

Value::Value(const Value& other) : kind(other.kind) {
    // this is the largest struct of the union
    // does this work?
    switch(kind) {
        case Obj:
            obj = other.obj;
            break;
        case Sequence:
            seq.elems = (Value*) malloc(other.seq.num * sizeof(Value));
            // memcpy(seq.elems, other.seq.elems, other.seq.num * sizeof(Value));
            for(u32 i = 0; i < other.seq.num; ++i)
                seq.elems[i] = other.seq.elems[i];
            seq.num = other.seq.num;
            break;
        case Integer:
            i = other.i;
            break;
        case Float:
            f = other.f;
            break;
        case Boolean:
            b = other.b;
            break;
        default:
            break;
    }
}

Value Value::operator= (const Value& other) {
    kind = other.kind;
    switch(kind) {
        case Obj:
            obj = other.obj;
            break;
        case Sequence:
            seq.elems = (Value*) malloc(other.seq.num * sizeof(Value));
            // memcpy(seq.elems, other.seq.elems, other.seq.num * sizeof(Value));
            for(u32 i = 0; i < other.seq.num; ++i)
                seq.elems[i] = other.seq.elems[i];
            seq.num = other.seq.num;
            break;
        case Integer:
            i = other.i;
            break;
        case Float:
            f = other.f;
            break;
        case Boolean:
            b = other.b;
            break;
        default:
            break;
    }
    return *this;
}

// Value::Value(Value&& other) : kind(other.kind) {
//     switch(kind) {
//         case Obj:
//             obj = other.obj;
//             other.obj = nullptr;
//             break;
//         case Sequence:
//             seq.elems = other.seq.elems;
//             seq.num = other.seq.num;
//             other.seq.elems = nullptr;
//             other.seq.num = 0;
//             break;
//         default:
//             // this is a copy of the bits which will
//             // preserve the value of a float or boolean
//             // or chacter.
//             i = other.i;
//     }
// }

Value::~Value() {
    if(is_sequence())
        delete seq.elems;
    // else if(is_object())
    //     delete o;
}

std::ostream& operator <<(std::ostream& out, const Value& val) {
    switch(val.kind) {
        case Integer:
            out << val.i;
            break;
        case Float:
            out << val.f;
            break;
        case Boolean:
            out << (val.b ? "true" : "false");
            break;
        case Nill:
            out << "nill";
            break;
        case Function:
            break;
        case Obj:
            // print object type thing
            out << "<" << val.obj->name() << " : " << val.obj  << ">";
            break;

        case Sequence:
            out << '[';
            if(val.seq.num != 0) {
                for(u32 i = 0; i < val.seq.num - 1; ++i)
                    out << val.seq.elems[i] << ", ";
                out << val.seq.elems[val.seq.num - 1];
            }
            out << ']';

    }
    return out;
}

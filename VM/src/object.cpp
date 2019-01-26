#include "object.hpp"
#include <cstdlib>


ObjString::ObjString(const std::string& str) : str(str) {

}

ObjString::ObjString(ObjString&& other) : str(std::move(other.str)){

}

ObjKind ObjString::kind() {
    return StringObj;
}

const std::string& ObjString::name() {
    return "string";
}

ObjLambda::ObjLambda(ObjString* name, Chunk chunk, u32 arity, u32 num_locals) : n(name),
    chunk(chunk), arity(arity), num_locals(num_locals) {
}

ObjKind ObjLambda::kind() {
    return LambdaObj;
}

const std::string& ObjLambda::name() {
    return "lambda";
}
ObjLambda::~ObjLambda() {
    delete n;
}

ObjStruct::ObjStruct(Value* mems, u32 num) : num(num) {
    members = new Value[num];
    for(u32 i = 0; i < num; ++i)
        members[i] = mems[i];
}

ObjKind ObjStruct::kind() {
    return StructObj;
}

const std::string& ObjStruct::name() {
    return "struct";
}

ObjStruct::~ObjStruct() {
    delete members;
}

Value* ObjStruct::attr(u32 i) {
    if(i < num) {
        return &members[i];
    }
    return nullptr;
}

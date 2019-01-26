#pragma once

#include "chunk.hpp"
#include "value.hpp"
#include <vector>
#include <string>

enum ObjKind {
    StringObj,
    LambdaObj,
    StructObj,
    // ObjUpValue, ?
};

struct Object {
    virtual ~Object() = default;
    Object* next;
    virtual ObjKind kind() = 0;
    virtual const std::string& name() = 0;
};


struct ObjString : public Object {
    ObjString(const std::string& str);
    ObjString(ObjString&& other);

    std::string str;
    virtual ObjKind kind();
    virtual const std::string& name();
};

struct ObjLambda : public Object {
    ObjString* n;
    Chunk chunk;
    u32 arity;
    u32 num_locals{0};


    ObjLambda(ObjString* name, Chunk chunk, u32 arity, u32 num_locals = 0);
    virtual ~ObjLambda();
    virtual ObjKind kind();
    virtual const std::string& name();
};

struct ObjStruct : public Object  {
    Value* members;
    u32 num;

    ObjStruct(Value* mems, u32 num);
    virtual ~ObjStruct();

    Value* attr(u32 i);

    virtual ObjKind kind();
    virtual const std::string& name();
};

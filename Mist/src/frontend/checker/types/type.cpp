//
// Created by Andrew Bregger on 2019-01-03.
//

#include "type.hpp"
#include <functional>
#include <algorithm>
#include <numeric>
#include <sstream>
#include "interpreter.hpp"


mist::Type::Type(mist::TypeKind k, u64 sz) : k(k), sz(sz) {

}

mist::PrimitiveType::PrimitiveType(mist::PrimitiveKind t, struct mist::String *s, u32 sz) :  Type(Primitive, sz), pk(t), str(s) {
}



mist::StructType::StructType(struct String *n, const std::unordered_map<struct String *, Type *> &m, u64 sz) :
        Type(Struct, sz), n(n), m(m) {

}

//const std::vector<mist::DeclInfo *> & mist::StructType::members() {
//    return m;
//}

mist::Type *mist::StructType::member(u32 index) {
    return nullptr;
}

mist::ClassType::ClassType(struct String *n, const std::unordered_map<struct String *, Type *> &m,
                           const std::unordered_map<struct String *, Type *> &mf, u64 sz) : StructType(n, m, sz), mf(mf) {
}

//std::vector<mist::DeclInfo *> mist::ClassType::methods() {
//    return mf;
//}

//const mist::DeclInfo *mist::ClassType::method(u32 index) {
//    return mf.at(index);
//}

mist::TupleType::TupleType(const std::vector<Type *> &elements, u64 sz) : Type(Tuple, sz), elements(elements) {
}

mist::Type *mist::TupleType::get(u32 index) {
    return elements.at(index);
}

u32 mist::TupleType::num() {
    return (u32) elements.size();
}

mist::VariantType::VariantType() : Type(Variant, 0) {

}

struct mist::String *mist::VariantType::name() {
    return n;
}

mist::FunctionType::FunctionType(mist::TupleType *p, mist::TupleType *r) : Type(Function, 8), p(p), r(r) {
}

mist::Type *mist::FunctionType::param(u32 index) {
    return p->get(index);
}

mist::Type *mist::FunctionType::returns(u32 index) {
    return r->get(index);
}

mist::TupleType *mist::FunctionType::params() {
    return p;
}

mist::TupleType *mist::FunctionType::returnss() {
    return r;
}

u32 mist::FunctionType::num_params() {
    return p->num();
}

u32 mist::FunctionType::num_returns() {
    return r->num();
}


std::string mist::VariantType::to_string() {
    return Type::to_string();
}

mist::ReferenceType::ReferenceType(mist::Type *b) : Type(Reference, 8), b(b) {
}

mist::Type *mist::ReferenceType::base() {
    return b;
}

mist::ArrayType::ArrayType(mist::Type *b, u64 length) : Type(Array, b->size() * length), b(b), l(length) {
}

mist::Type *mist::ArrayType::base() {
    return b;
}

u64 mist::ArrayType::length() {
    return l;
}

mist::PointerType::PointerType(mist::Type *b) : Type(Pointer, 8), b(b) {
}

mist::Type *mist::PointerType::base() {
    return b;
}

mist::MutableType::MutableType(mist::Type *b) : Type(Mutable, b->size()), b(b) {
}

mist::Type *mist::MutableType::base() {
    return b;
}

mist::UnitType::UnitType() : Type(Unit, 0) {

}


std::string mist::PrimitiveType::to_string() {
    return name()->value();
}


std::string mist::StructType::to_string() {
    return name()->value();
}

std::string mist::ClassType::to_string() {
    return StructType::to_string();
}


std::string mist::TupleType::to_string() {
    std::stringstream ss;
    ss << "(";
    for(auto& t : this->elements) {
        ss << t->to_string();
        ss << ", ";
    }

    u32 pos = (u32) ss.tellp();
    ss.seekp(pos - 2);
    ss << ")";

    return ss.str();
}

std::string mist::ReferenceType::to_string() {
    return "ref " + base()->to_string();
}

std::string mist::FunctionType::to_string() {
    std::stringstream ss;
    ss << p->to_string() << " -> " << r->to_string();
    return ss.str();
}

std::string mist::MutableType::to_string() {
    return "mut " + base()->to_string();
}

std::string mist::ArrayType::to_string() {
    return "[" + std::to_string(l) + "]" + base()->to_string();
}

std::string mist::PointerType::to_string() {
    return "*" + base()->to_string();
}


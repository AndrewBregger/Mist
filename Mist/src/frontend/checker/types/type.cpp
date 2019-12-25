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



mist::StructType::StructType(struct String *n, const std::unordered_map<struct String *, Type *> &m, DeclInfo* info, u64 sz) :
        Type(Struct, sz), n(n), m(m), info(info) {

}

//const std::vector<mist::DeclInfo *> & mist::StructType::members() {
//    return m;
//}

mist::Type *mist::StructType::member(u32 index) {
    return nullptr;
}

void mist::StructType::set_infos(const std::vector<mist::DeclInfo *>& infos) {
    memberInfos = infos;
}

mist::ClassType::ClassType(struct String *n, const std::unordered_map<struct String *, Type *> &m,
                           const std::unordered_map<struct String *, Type *> &mf, DeclInfo* info, u64 sz) : StructType(n, m, info, sz), mf(mf) {
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

mist::FunctionType::FunctionType(TupleType *p, Type *r, DeclInfo *info) : Type(Function, 8), p(p), r(r), info(info) {
}

mist::Type *mist::FunctionType::param(u32 index) {
    return p->get(index);
}

mist::Type *mist::FunctionType::returns() {
    return r;
}

mist::TupleType *mist::FunctionType::params() {
    return p;
}

u32 mist::FunctionType::num_params() {
    return p->num();
}

//u32 mist::FunctionType::num_returns() {
//    return r->num();
//}


std::string mist::VariantType::to_string(bool) {
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

mist::ListType::ListType(const std::vector<mist::Type *> &subtypes, u64 sz) : Type(List, sz), subtypes(subtypes) {
    if(subtypes.size() < 2) {
        throw std::runtime_error("invalid number of list subtypes");
    }
}


std::string mist::PrimitiveType::to_string(bool) {
    return name()->value();
}


std::string mist::StructType::to_string(bool) {
    return name()->value();
}

std::string mist::ClassType::to_string(bool) {
    return StructType::to_string();
}


std::string mist::TupleType::to_string(bool variant) {
    std::stringstream ss;
    if(!variant)
        ss << "(";

    if(!elements.empty()) {
        for (auto iter = this->elements.begin(); iter < this->elements.end() - 1; ++iter) {
            ss << (*iter)->to_string(variant);
            ss << ", ";
        }

        ss << (*(this->elements.end() - 1))->to_string(variant);
    }

    if(!variant) {
        ss << ")";
    }

    return ss.str();
}

std::string mist::ReferenceType::to_string(bool) {
    return "ref " + base()->to_string();
}

std::string mist::FunctionType::to_string(bool) {
    std::stringstream ss;
    ss << p->to_string() << " -> " << r->to_string();
    return ss.str();
}

std::string mist::MutableType::to_string(bool) {
    return "mut " + base()->to_string();
}

std::string mist::ArrayType::to_string(bool) {
    return "[" + std::to_string(l) + "]" + base()->to_string();
}

std::string mist::PointerType::to_string(bool) {
    return "*" + base()->to_string();
}

std::string mist::ListType::to_string(bool) {
    std::stringstream ss;
    for(u32 i = 0; i < subtypes.size() - 1; ++i)
        ss << subtypes[i]->to_string() << ", ";
    ss << subtypes.back()->to_string();
    return ss.str();
}

std::string mist::UnitType::to_string(bool) {
    return "Unit";
}
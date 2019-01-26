//
// Created by Andrew Bregger on 2019-01-03.
//

#pragma once

#include "common.hpp"
#include <unordered_map>

namespace mist {

    class TupleType;

    struct String;

    struct DeclInfo;

//    enum Mutability {
//        Immutable,
//        MutableType,
//    };

    enum TypeKind {
        Primitive,
        Struct,
        Class,
        Variant,
        Function,
        Tuple,
        Map,
        String,
        Array,
        Pointer,
        Reference,
        Unit,

        Mutable,
        /*
            Generic,
            TypeClass,
        */
    };

    enum PrimitiveKind {
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        Char,
        Bool,
//        Atom,
    };

    class Type {
        TypeKind k;
        u64 sz;
        std::string str_cache;

    public:

        Type(TypeKind k, u64 sz);

        /// Returns the type kind of this type
        inline auto kind() { return k; }

        /// Returns the size of this type
        inline auto size() { return sz; }

        virtual Type* base_type() { return this; }


        virtual bool is_primative() { return false; }

        virtual bool is_named_type() { return false; }

        virtual bool is_function() { return false; }

        virtual bool is_arithmetic_type() { return true; }

        virtual bool is_mutable() { return false; }

        virtual bool is_pointer() { return false; }

        virtual bool is_reference() { return false; }

        virtual std::string to_string() { return ""; }
    };


    class PrimitiveType : public Type {
        PrimitiveKind pk;               //< The primative this type represents
        struct String* str;             //< The string name of this type

    public:

        PrimitiveType(PrimitiveKind t, struct String* s, u32 sz);

        inline struct String* name() { return str; }

        inline PrimitiveKind pkind() { return pk; }

        virtual std::string to_string() override;

        virtual bool is_primative() override { return true; }

        virtual bool is_arithmetic_type() override { return pk != Bool && pk != Char; }

        bool is_integer() { return I8 <= pk && pk <= U64; }

        bool is_floating() { return F32 == pk || F64 == pk; }

        bool is_boolean() { return pk == Bool; }

        bool is_character() { return pk == Char; }
    };

    class StructType : public Type {
        struct String* n{nullptr};      //< Name of the struct
//        std::vector<DeclInfo*> m;       //< The Declaration info of this types member variables.
        std::unordered_map<struct String*, Type*> m;

    public:

        StructType(struct String *n, const std::unordered_map<struct String *, Type *> &m, u64 sz);

        inline struct String* name() { return n; }

        const std::vector<DeclInfo *> & members();

        Type* member(u32 index);

        virtual std::string to_string() override;

        virtual bool is_named_type() override { return true; }
    };

    class ClassType : public StructType {
//        std::vector<DeclInfo*> mf;      //< The methods of this class.
        std::unordered_map<struct String*, Type*> mf;

    public:
        ClassType(struct String *n, const std::unordered_map<struct String *, Type *> &m,
                  const std::unordered_map<struct String *, Type *> &mf, u64 sz);

        const std::vector<DeclInfo*>& methods();

        const DeclInfo* method(u32 index);

        virtual std::string to_string() override;
    };

    class VariantType : public Type {
        struct String* n{nullptr};

        std::vector<DeclInfo*> members;

    public:

        VariantType();

        struct String* name();

        virtual std::string to_string() override;

        virtual bool is_named_type() override { return true; }
    };

    class FunctionType : public Type {
        TupleType* p;
        TupleType* r;


    public:

        FunctionType(TupleType* p, TupleType* r);

        Type* param(u32 index);

        Type* returns(u32 index);

        TupleType* params();

        TupleType* returnss();

        u32 num_params();

        u32 num_returns();

        virtual std::string to_string() override;

        virtual bool is_function() override { return true; }
    };

    class TupleType : public Type {
        std::vector<Type*> elements;

    public:

        TupleType(const std::vector<Type *> &elements, u64 sz);

        Type* get(u32 index);

        u32 num();

        virtual std::string to_string() override;
    };

    // maybe this should be a special loaded from
    // a special load file that contains the string and map implementation.
    // this would allow for map and string type to only need to point to the
    // structure representation.
    // I will think about it.
    class MapType : public Type {
//        Type* key;
//        Type* value;
    };

    class StringType : public Type {
    };


    class ArrayType : public Type {
        Type* b;
        u64 l;

    public:
        ArrayType(Type* b, u64 length);

        Type* base();

        u64 length();

        virtual std::string to_string() override;
    };

    class ReferenceType : public Type {
        Type* b;

    public:
        ReferenceType(Type* b);

        Type* base();

        virtual std::string to_string() override;

        virtual bool is_reference() override { return true; }
    };

    class PointerType : public Type {
        Type* b;

    public:
        PointerType(Type* b);

         Type* base();

        virtual std::string to_string() override;

        virtual bool is_pointer() override { return true; }
    };

    class MutableType : public Type {
        Type* b;

    public:
        MutableType(Type* b);

        Type* base();

        virtual Type* base_type() override { return b; }

        virtual std::string to_string() override;

        virtual bool is_mutable() override { return true; }
    };

    class UnitType : public Type {
    public:
        UnitType();
    };

}

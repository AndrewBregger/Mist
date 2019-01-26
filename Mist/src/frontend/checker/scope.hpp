//
// Created by Andrew Bregger on 2018-12-29.
//

#pragma once
#include <unordered_map>
#include <unordered_set>

//#include "interpreter.hpp"



// It might be more advantagous to build a struct like DeclInfo



namespace mist {
    struct DeclInfo;
    class Interpreter;
    struct String;

    enum ScopeKind {
        PreludeScope,
        ModuleScope,
        VariantScope,
        TypeScope,
        BlockScope,
        ParamScope,
    };

    class Scope {
    public:
        Scope(Scope* parent, ScopeKind  kind, Interpreter* interp);

        inline ScopeKind kind() { return k; }

        Scope *create_child(ScopeKind kind);

        bool is_child(Scope* scope);

        void add_child(Scope *scope);

        bool add(struct mist::String *name, DeclInfo *info);

        bool contains(struct mist::String* name);

        DeclInfo* find(struct mist::String* name);

        bool local_contains(struct mist::String* name);

        DeclInfo* local_find(struct mist::String* name);

        Scope* parent{nullptr};

    private:
        Interpreter* interp{nullptr};
        ScopeKind k;

        std::unordered_map<struct mist::String*, DeclInfo*> elements;
        std::unordered_set<Scope*> children;
    };
}

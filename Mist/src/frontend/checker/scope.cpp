//
// Created by Andrew Bregger on 2018-12-29.
//

#include "scope.hpp"

#include "interpreter.hpp"

mist::Scope::Scope(Scope* parent, ScopeKind  kind, mist::Interpreter *interp) : interp(interp), k(kind), parent(parent) {
}

mist::Scope *mist::Scope::create_child(ScopeKind kind) {
    auto child = new Scope(this, kind, interp);
    this->add_child(child);
    return child;
}

bool mist::Scope::is_child(mist::Scope *scope) {
    return children.find(scope) != children.end();
}

void mist::Scope::add_child(Scope *scope) {
    children.insert(scope);
}

bool mist::Scope::add(struct mist::String *name, DeclInfo *info) {
    std::cout << "Scope Adding: " << name->value() << " at " << info << std::endl;
    if(local_contains(name))
        return false;

    elements.emplace(name, info);
    return true;
}

bool mist::Scope::contains(struct mist::String *name) {
    Scope* current = this;
    while(current) {
        if(current->local_contains(name))
            return true;
        current = current->parent;
    }
    return false;
}

mist::DeclInfo *mist::Scope::find(struct mist::String *name) {
    Scope* current = this;
    DeclInfo* info = nullptr;
    while(current) {
        info = current->local_find(name);
        if(info)
            break;
        current = current->parent;
    }
    return info;
}

bool mist::Scope::local_contains(struct mist::String *name) {
    return elements.find(name) != elements.end();
}

mist::DeclInfo *mist::Scope::local_find(struct mist::String *name) {
    auto iter = elements.find(name);
    if(iter != elements.end())
        return iter->second;
    return nullptr;
}

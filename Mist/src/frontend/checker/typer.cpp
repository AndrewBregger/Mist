    //
// Created by Andrew Bregger on 2018-12-29.
//

#include "typer.hpp"

#include "frontend/parser/ast/ast_printer.hpp"

#define CAST_PTR(Type, expr) static_cast<Type*>(expr)
#define CAST(Type, expr) static_cast<Type>(expr)

mist::DeclInfo::DeclInfo(struct String *name, i32 index, mist::Scope *scope, ast::Decl *decl)
        : state(Unresovled), decl(decl), decl_index(index),
          name(name), k(Unknown), scope(scope) {

    if(k == Func)
        addressing = Address;
}

bool mist::DeclInfo::is_type() {
    return k == Ty;
}

bool mist::DeclInfo::is_local() {
    return k == Ty;
}

bool mist::DeclInfo::is_variable() {
    return is_local();
}

bool mist::DeclInfo::is_function() {
    return k == Func;
}

bool mist::DeclInfo::is_struct() {
    return is_type() && decl->kind() == ast::Struct;
}

bool mist::DeclInfo::is_class() {
    return is_type() && decl->kind() == ast::Class;
}

bool mist::DeclInfo::is_typeclass() {
    return is_type() && decl->kind() == ast::TypeClass;
}

bool mist::DeclInfo::is_resolved() {
    return state == Resolved;
}

bool mist::DeclInfo::resolving() {
    return state == Resolving;
}

mist::Typer::Typer(mist::Interpreter *interp) : interp(interp), prelude(new Scope(nullptr, PreludeScope, interp)) {
}

void mist::DeclInfo::resolve(mist::Type *type, mist::Addressing addr, DeclInfoKind k) {
    this->type = type;
    this->addressing = addr;
    this->state = Resolved;
    this->k = k;
}

    void mist::Typer::create_root() {
    root = new Scope(prelude, ModuleScope, interp);
    current = root;
}

std::vector<mist::DeclInfo *> mist::Typer::add_pattern(ast::Pattern *pattern, ast::Decl *decl, i32 index) {
    return std::vector<DeclInfo*>();
//    std::vector<mist::DeclInfo *> infos;
//    switch(pattern->kind()) {
//        case ast::IdentPatKind: {
//            auto pat = static_cast<ast::IdentPat*>(pattern);
//            auto info = new DeclInfo(pat->name->value, index, current, decl);
//            if(add_name(pat->name, info))
//               infos.push_back(info);
//        }
//        case ast::TuplePatKind: {
//            auto pat = static_cast<ast::TuplePat*>(pattern);
//            for(int i = 0; i < pat->elements.size(); ++i) {
//                if(add_pattern(pat->elements[i], decl, i))
//                    infos.push_back(info);
//            }
//
//        } break;
//        case ast::StructurePatKind: {
//            auto pat = static_cast<ast::StructPat*>(pattern);
//            for(int i = 0; i < pat->members.size(); ++i) {
//                if(!add_pattern(pat->members[i], decl, i))
//                    return false;
//            }
//
//        } break;
//        case ast::VariantPatKind: {
//            auto pat = static_cast<ast::VariantPat*>(pattern);
//            for(int i = 0; i < pat->members.size(); ++i) {
//                if(!add_pattern(pat->members[i], decl, i))
//                    return false;
//            }
//
//        } break;
//        case ast::ListPatKind: {
//            auto pat = static_cast<ast::ListPat*>(pattern);
//            for(int i = 0; i < pat->patterns.size(); ++i) {
//                if(!add_pattern(pat->patterns[i], decl, i))
//                    return false;
//            }
//        } break;
//        case ast::UnderscorePatKind:
//            break;
//        default:
//            interp->report_error(pattern->pos(), "invalid pattern being added to scope");
//            break;
//    }
//    return false;
}

bool mist::Typer::add_name(ast::Ident *name, mist::DeclInfo *info) {
    if(current->add(name->value, info))
        return true;

    /// get the
    interp->report_error(name->pos, "redeclaration of name: '%s'", name->value->val.c_str());
    return false;
}

bool mist::Typer::add_name(ast::Ident *name, ast::Decl *decl) {
    auto info = new DeclInfo(nullptr, -1, current, decl);
    return add_name(name, info);
}

mist::Scope * mist::Typer::push_scope(ScopeKind kind) {
    current = current->create_child(kind);
    return current;
}

void mist::Typer::pop_scope() {
    current = current->parent;
}

mist::Type *mist::Typer::resolve_expr(ast::Expr *expr) {
    if(!expr) {
        std::clog << "[LOG]" << "null expression" << std::endl;
        return nullptr;
    }
    Type* type = nullptr;
    switch(expr->kind()) {
        case ast::UnitLit:
            return type_unit;
        case ast::StringConst:
            interp->report_error(expr->pos(), "Strings are not implemented at this time");
            return nullptr;
        case ast::IntegerConst:
        case ast::CharConst:
        case ast::FloatConst:
            type = check_integral_literal_and_type_suffix(expr);
            break;
        case ast::BooleanConst:
            type = type_bool;
            break;
        case ast::Unary:
            type = resolve_unary_expr(CAST_PTR(ast::UnaryExpr, expr));
            break;
        case ast::Binary:
            type = resolve_binary_expr(CAST_PTR(ast::BinaryExpr, expr));
            break;
        case ast::Value:
            type = resolve_value(CAST_PTR(ast::ValueExpr, expr));
            break;
        case ast::Tuple: {
            auto tup = CAST_PTR(ast::TupleExpr, expr);
            std::vector<Type*> elements;
            u64 size = 0;
            for(auto elems : tup->values) {
                auto t = resolve_expr(elems);
                if(!t)
                    break;
                elements.push_back(t);
                size += t->size();
            }
            Type* tupletype = new TupleType(elements, size);
            type = interp->find_type(tupletype);
            if(type)
                delete tupletype;
            else {
                type = interp->add_type(tupletype);
            }
            break;
        }
        case ast::Block:
            return resolve_block(CAST_PTR(ast::BlockExpr, expr));
        case ast::DeclDecl: {
            auto decl = CAST_PTR(ast::DeclExpr, expr);
            resolve_decl(decl->decl);
            return type_unit;
        }
        case ast::Parenthesis:
        case ast::Selector:
        default:
            break;

    }

    expr->t = type;
    return type;
}

mist::Type *mist::Typer::resolve_typespec(ast::TypeSpec *spec) {
    Type* type = nullptr;
    switch(spec->k) {
        case ast::Named: {
            auto name = CAST_PTR(ast::NamedSpec, spec);
            DeclInfo* info = resolve_name(name->name);
            if(info->is_resolved()) {
                if(info->is_type()) {
                    return info->type;
                }
                else {
                    interp->report_error(spec->pos(), "'%s' does not refer to a type", name->name->value->val.c_str());
                }
            }
            else {
                if(info->scope->kind() == ModuleScope)
                    return resolve_toplevel_decl(info);
                else {
                    interp->report_error("Cyclic reference, I think");
                }
            }
        } break;
        case ast::TupleType: {
            auto name = CAST_PTR(ast::TupleSpec, spec);
            std::vector<Type*> specs;

        } break;
        case ast::FunctionType: {
            auto name = CAST_PTR(ast::FunctionSpec, spec);

        } break;
        case ast::TypeClassType: {
            auto name = CAST_PTR(ast::TypeClassSpec, spec);

        } break;
        case ast::Array: {
            auto name = CAST_PTR(ast::ArraySpec, spec);

        } break;
//        case ast::DynamicArray: {
//            auto name = CAST_PTR(ast::DySpec, spec);
//
//        }
        case ast::Map: {
            auto name = CAST_PTR(ast::MapSpec, spec);

        } break;
        case ast::Pointer: {
            auto name = CAST_PTR(ast::PointerSpec, spec);

        } break;
        case ast::Reference: {
            auto name = CAST_PTR(ast::ReferenceSpec, spec);

        } break;
//        case ast::TypeSpecKind::MutableType: {
//            auto name = CAST_PTR(ast::MutableSpec, spec);
//
//        }
        case ast::Path: {
            auto name = CAST_PTR(ast::PathSpec, spec);
            const auto& path = name->path;

            if(path.size() == 1) {
                return resolve_typespec(path.front());
            }
            else {
                interp->report_error("Type paths are unimplmented");
                return nullptr;
            }
        } break;
        case ast::Unit: {
            return type_unit;
        } break;
    }
    return nullptr;
}


//std::vector<mist::DeclInfo *> mist::Typer::resolve_local(ast::LocalDecl *local) {
////    std::vector<Type*> expected_types;
////    for(auto t : local->sp) {
////        auto ty = resolve_typespec(t);
////        expected_types.push_back(ty);
////    }
////
////    std::vector<Type*> expr_types;
////    for(auto expr : local->expr()) {
////        auto type = resolve_expr(expr);
////        expr_types.push_back(type);
////    }
////
////    auto declInfo = add_pattern(local->name, local, 0);
////
////
////    if(resolve_expected_types_and_initialization(local->name, declInfo, expected_types, expr_types))
////        return declInfo;
////    else
////        return std::vector<DeclInfo*>();
//}



mist::DeclInfo *mist::Typer::resolve_name(ast::Ident *name) {
    return resolve_name(current, name, false);
}

mist::DeclInfo *mist::Typer::resolve_name(mist::Scope *scope, ast::Ident *name, bool only_scope) {
    DeclInfo* info = nullptr;
    std::cout << "Resolving Name: ";
    std::cout << name->value->val << std::endl;

    if(only_scope)
        info = scope->local_find(name->value);
    else
        info = scope->find(name->value);

    return info;
}

//bool mist::Typer::resolve_local_pattern(ast::Pattern *pattern, mist::DeclInfo *info, mist::Type *expectedType,
//                                    mist::Type *exprType) {
//
//    switch(pattern->kind()) {
//        case ast::IdentPatKind: {
//            auto pat = static_cast<ast::IdentPat*>(pattern);
//        }
//        case ast::TuplePatKind: {
//            auto pat = static_cast<ast::TuplePat*>(pattern);
//
//        } break;
//        case ast::StructurePatKind: {
//            auto pat = static_cast<ast::StructPat*>(pattern);
//
//        } break;
//        case ast::VariantPatKind: {
//            auto pat = static_cast<ast::VariantPat*>(pattern);
//
//        } break;
//        case ast::ListPatKind: {
//            auto pat = static_cast<ast::ListPat*>(pattern);
//        } break;
//        case ast::UnderscorePatKind:
//            break;
//        default:
//            break;
//    }
//    return false;
//}
//
//bool mist::Typer::resolve_expected_types_and_initialization(ast::Pattern *pattern,
//                                                        const std::vector<mist::DeclInfo *> &info,
//                                                        const std::vector<mist::Type *> &expectedTypes,
//                                                        const std::vector<mist::Type *> &exprTypes) {
//    switch(pattern->kind()) {
//        case ast::IdentPatKind: {
//            auto pat = static_cast<ast::IdentPat*>(pattern);
//            // if the pattern is simply an identifier then the rhs must be a single
//            // expression
//
//        }
//        case ast::TuplePatKind: {
//            auto pat = static_cast<ast::TuplePat*>(pattern);
//
//        } break;
//        case ast::StructurePatKind: {
//            auto pat = static_cast<ast::StructPat*>(pattern);
//
//        } break;
//        case ast::VariantPatKind: {
//            auto pat = static_cast<ast::VariantPat*>(pattern);
//
//        } break;
//        case ast::ListPatKind: {
//            auto pat = static_cast<ast::ListPat*>(pattern);
//        } break;
//        case ast::UnderscorePatKind:
//            break;
//        default:
//            break;
//    }
//    return false;
//}

void mist::Typer::resolve_module(ast::Module *module) {
    /// Add all of the top level symbols to the symbol table.
    /// this is to allow for out of order use of toplevel names.


    std::vector<DeclInfo*> declinfos;
    for(auto decl : module->toplevelDeclarations) {
        if(decl->kind() == ast::Local) {
            interp->report_error(decl->pos, "Compiler Error: Invalid toplevel declaration");
            return;
        }
        // because of the check above, all of the names are set for the declaration so no other processing
        // with be needed.
        auto info = new DeclInfo(decl->name->value, -1, current, decl);

        add_name(decl->name, info);
        declinfos.push_back(info);
    }

    for(auto info : declinfos) {
        resolve_toplevel_decl(info);
    }
}

mist::Type *mist::Typer::resolve_toplevel_decl(mist::DeclInfo *info) {
    switch(info->decl->kind()) {
        case ast::Global:
            return resolve_global_decl(CAST_PTR(ast::GlobalDecl, info->decl), info);
        case ast::Struct:
            return resolve_struct_decl(CAST_PTR(ast::StructDecl, info->decl), info);
        case ast::Class:
            return resolve_class_decl(CAST_PTR(ast::ClassDecl, info->decl), info);
        case ast::TypeClass:
        case ast::Function:
            return resolve_function_decl(CAST_PTR(ast::FunctionDecl, info->decl), info);
        case ast::OpFunction:
        case ast::Use:
        case ast::Impl:
        case ast::Generic:
        case ast::Variant:
        case ast::VariantMember:
        case ast::Alias:
        case ast::Prelude:
            interp->report_error("Compiler Error: Attempting to resolve a prelude declaration");
            break;
        default:
            interp->report_error(info->decl->pos, "Invalid declaration at toplevel of module");
            return nullptr;
    }
}

mist::Type *mist::Typer::resolve_global_decl(ast::GlobalDecl *decl, mist::DeclInfo *info) {
#ifdef DEBUG
    std::cout << __FUNCTION__ << std::endl;
#endif
    auto typespec = decl->tp;
    auto init = decl->init;

    Type* type = nullptr,* init_type = nullptr;

    if(typespec)
        type = resolve_typespec(typespec);

    if(init)
        init_type = resolve_expr(init);

    if(!init_type && !type) {
       // this should have been caught when parsing.
       interp->report_error(decl->pos, "declaration must be given a type or initialization expression,"
                                       "this declaration has neither");

       info->state = Invalid;
       return nullptr;
    }


    Type* res = nullptr;
    // if both are given, then check if they are compatable, otherwise
    if(init_type && type) {
#ifdef DEBUG
            std::cout << "\tinit_type: " << init_type->to_string() << std::endl
                      << "\ttype: " << type->to_string() << std::endl;
#endif

        if(equivalent_type(init_type, type)) {
            res =  init_type;
        }
        else {
            interp->report_error(init->pos(), "incompatable types: found '%s', expecting: '%s'", init_type->to_string().c_str(), type->to_string().c_str());
            return nullptr;
        }
    }
    else if(type)
        res = type;
    else if(init_type)
        res = init_type;

    Addressing addr = addressing_from_type(res);
    info->resolve(res, addr, Variable);

#ifdef DEBUG
    std::cout << "Result Type; " << res->to_string() << " addressing: "  << addr << std::endl;
#endif

    info->type = res;
//    decl->t = res;

    return res;
}


/// Assumes the name has already been added to the namespace
mist::Type *mist::Typer::resolve_struct_decl(ast::StructDecl *decl, mist::DeclInfo *info) {
    // this resolves the name of the decl to make sure it can be
    // declared. It also adds it to the scope.
    if(add_name(decl->name, info)) {
        for(auto field : decl->fields) {
        }
    }
    return nullptr;
}

/// Assumes the name has already been added to the namespace
mist::Type *mist::Typer::resolve_class_decl(ast::ClassDecl *decl, mist::DeclInfo *info) {
    return nullptr;
}

/// Assumes the name has already been added to the namespace
mist::Type *mist::Typer::resolve_function_decl(ast::FunctionDecl *decl, mist::DeclInfo *info) {
    Type* functionType = nullptr;
    TupleType* paramType = nullptr;
    TupleType* returnType = nullptr;

    //for now I am assuming there are no parameters and the function returns Unit
    std::vector<Type*> params;
    std::vector<Type*> returns;
    u64 paramsSize = 0;
    u64 returnsSize = 0;

    push_scope(ParamScope);
    for(auto param : decl->parameters) {
        auto pinfo = resolve_local_decl(param);
        params.push_back(info->type);
        paramsSize += info->type->size();
    }

    for(auto ret : decl->returns) {
        auto type = resolve_typespec(ret);
        returns.push_back(type);
        returnsSize += type->size();
    }

    if(decl->returns.size() == 0) {
        returns.push_back(type_unit);
    }

    if(params.empty())
        paramType = CAST_PTR(TupleType, type_emptytuple);
    else
        paramType = new TupleType(params, paramsSize);

    if(returns.empty())
        returnType = CAST_PTR(TupleType, type_emptytuple);
    else
        returnType =  new TupleType(returns, returnsSize);

    functionType = new FunctionType(paramType, returnType);

    auto body_result = resolve_expr(decl->body);

    pop_scope();

    if(!equivalent_type(type_unit, body_result)) {
        interp->report_error(decl->pos, "incompatable return type");
        return nullptr;
    }

    auto found_type = interp->find_type(functionType);
    if(found_type) {
        delete functionType;
        functionType = found_type;
    }
    else {
        interp->add_type(functionType);
    }

    info->type = functionType;
    info->resolve(functionType, Address, Func);
    return functionType;
}


mist::Type *mist::Typer::resolve_impl_decl(ast::ImplDecl *decl, mist::DeclInfo *info) {
    return nullptr;
}

void mist::Typer::resolve_decl(ast::Decl *decl) {
    switch(decl->kind()) {
        case ast::Local: {
            resolve_local_decl(CAST_PTR(ast::LocalDecl, decl));
        } break;
        case ast::Struct: {
            auto info = new DeclInfo(decl->name->value, -1, current, decl);
            resolve_struct_decl(CAST_PTR(ast::StructDecl, info->decl), info);
        } break;
        case ast::Class: {
            auto info = new DeclInfo(decl->name->value, -1, current, decl);
            resolve_class_decl(CAST_PTR(ast::ClassDecl, info->decl), info);
        } break;
        case ast::TypeClass:
        case ast::Function:
        case ast::OpFunction:
        case ast::Use:
        case ast::Impl:
        case ast::Generic:
        case ast::Variant:
        case ast::VariantMember:
        case ast::Alias:
        case ast::Prelude:
        case ast::Global:
        default:
            break;
    }
}

mist::DeclInfo * mist::Typer::resolve_local_decl(ast::LocalDecl *decl) {
    // resolves type annotations and initialization expressions
    std::vector<Type*> initTypes;
    for(auto expr : decl->init) {
        auto type = resolve_expr(expr);
        if(type)
            initTypes.push_back(type);
        else
            return nullptr;
    }

    std::vector<Type*> annoTypes;
    for(auto spec : decl->sp) {
        auto type = resolve_typespec(spec);
        if(type)
            annoTypes.push_back(type);
        else
            return nullptr;
    }

    // resolves the pattern and matches it with the appropriate types
    // doing the simple case for now

    // this is enough to resolve functions and structures
    if(decl->name->kind() == ast::IdentPatKind) {
        auto ident = CAST_PTR(ast::IdentPat, decl->name);
        if(initTypes.size() == 0) {
            if(annoTypes.size() != 1) {
                interp->report_error(decl->sp.front()->pos(), "unable to match pattern with type annotation pattern");
            }
            else {
                auto info = new DeclInfo(ident->name->value, 0, current, decl);
                info->resolve(annoTypes.front(), addressing_from_type(annoTypes.front()), Variable);
                add_name(ident->name, info);
                return info;
            }
        }
        else {
            if(initTypes.size() > 1) {
                interp->report_error(decl->expr().front()->pos(), "expecting 1 initialization expression, found: '%u'", initTypes.size());
                return nullptr;
            }

            if(annoTypes.size() > 1) {
                interp->report_error(decl->expr().front()->pos(), "expecting 1 type annotation, found: '%u'", annoTypes.size());
                return nullptr;
            }

            if(annoTypes.size() == 0) {
                auto info = new DeclInfo(ident->name->value, 0, current, decl);
                info->resolve(initTypes.front(), addressing_from_type(initTypes.front()), Variable);
                add_name(ident->name, info);
                return info;
            }
            else {
               auto expected = annoTypes.front();
               auto init = initTypes.front();

               // we are ignoring the mutablility of the types
               if(equivalent_type(init, expected, true)) {
                   auto info = new DeclInfo(ident->name->value, 0, current, decl);
                   info->resolve(expected, addressing_from_type(expected), Variable);
                   add_name(ident->name, info);
                   return info;
               }
               else {
                   interp->report_error(decl->sp.front()->pos(), "expecting type: '%s', found type: '%s'", expected->to_string().c_str(), init->to_string().c_str());
               }
            }
        }
    }
    return nullptr;
}

// type1 is the computed types and type2 is the expected type
/// @TODO: Expand to handle sub-typing and derived types.
bool mist::Typer::equivalent_type(mist::Type *type1, mist::Type *type2, bool ignore_mut) {
    if(ignore_mut) {
        /// for most this returns the original type
        /// only for mut types does this return the underlying type.
        type1 = type1->base_type();
        type2 = type2->base_type();
    }

    if(type1 == type2)
        return true;

    /// add checks for derives

    if(type1->kind() != type2->kind()) {
        return false;
    }
    else {
       switch(type1->kind()) {
           case Primitive: {
               /// this is a very strict check for primitves, this could be changed to allow conversion
               /// between types.
                auto ptype1 = CAST_PTR(PrimitiveType, type1);
                auto ptype2 = CAST_PTR(PrimitiveType, type2);
                /// check for compatability between the primatives
                return ptype1->pkind() == ptype2->pkind();
           }
           case Struct: {
               auto p1 = CAST_PTR(StructType, type1);
               auto p2 = CAST_PTR(StructType, type2);

               return p1->name() == p2->name();
           }
           case Class: {
               auto p1 = CAST_PTR(ClassType, type1);
               auto p2 = CAST_PTR(ClassType, type2);
               return p1->name() == p2->name();
           }
           case Variant: {
               auto p1 = CAST_PTR(VariantType, type1);
               auto p2 = CAST_PTR(VariantType, type2);

               return p1->name() == p2->name();
           }
           case Function: {
               auto p1 = CAST_PTR(FunctionType, type1);
               auto p2 = CAST_PTR(FunctionType, type2);

               if(equivalent_type(p1->params(), p2->params()) &&
                  equivalent_type(p1->returnss(), p2->returnss()))
                   return true;
               return false;
           }
           case Tuple: {
               auto p1 = CAST_PTR(TupleType, type1);
               auto p2 = CAST_PTR(TupleType, type2);

               if(p1->num() != p2->num())
                   return false;

               for(u32 i = 0; i < p1->num(); ++i) {
                   if(!equivalent_type(p1->get(i), p2->get(i)))
                       return false;
               }

               return true;
           }
           case Map: {
               auto p1 = CAST_PTR(MapType, type1);
               auto p2 = CAST_PTR(MapType, type2);

               return false;
           }
           case mist::TypeKind::String: {
               return true;
           }
           case Array: {
               auto p1 = CAST_PTR(ArrayType, type1);
               auto p2 = CAST_PTR(ArrayType, type2);

               return p1->length() == p2->length() && equivalent_type(p1->base(), p2->base());
           }
           case Pointer: {
               auto p1 = CAST_PTR(PointerType, type1);
               auto p2 = CAST_PTR(PointerType, type2);

               return equivalent_type(p1->base(), p2->base());
           }
           case Reference: {
               auto p1 = CAST_PTR(ReferenceType, type1);
               auto p2 = CAST_PTR(ReferenceType, type2);

               return equivalent_type(p1->base(), p2->base());
           }
           case Mutable:
               auto p1 = CAST_PTR(MutableType, type1);
               auto p2 = CAST_PTR(MutableType, type2);
               return equivalent_type(p1->base(), p2->base());
       }
    }
}

mist::Type * mist::Typer::add_prelude(struct mist::String *name, mist::Type *type, Addressing addr, DeclInfoKind k) {
    auto declinfo = new DeclInfo(nullptr, -1, prelude, nullptr);
    declinfo->resolve(type, addr, k);
    prelude->add(name, declinfo);
    return type;
}

mist::Addressing mist::Typer::addressing_from_type(mist::Type *type) {
    // remove a mutable wrapper
    type = type->base_type();

    if(type->is_pointer() || type->is_reference() || type->is_function())
        return Address;

    return Value;
}

mist::Type *mist::Typer::check_integral_literal_and_type_suffix(ast::Expr *expr) {
                                                                             Type* literal_type = nullptr;
                                                                             auto suffix_type = ast::ConstantType::None;

                                                                             switch(expr->kind()) {
                                                                             case ast::IntegerConst: {
                                                                             auto lit = CAST_PTR(ast::IntegerConstExpr, expr);
                                                                             suffix_type = lit->cty;
                                                                             /// the default type for literals integer is i32
                                                                             literal_type = type_i32;
                                                                             } break;
                                                                             case ast::CharConst: {
                                                                             /// I thought i added the a type suffix to character literals. - Andrew B
                                                                             auto lit = CAST_PTR(ast::CharConstExpr, expr);
                                                                             //            suffix_type = lit->
                                                                             literal_type = type_char;
                                                                             return literal_type;
                                                                             } break;
                                                                             case ast::FloatConst: {
                                                                             auto lit = CAST_PTR(ast::FloatConstExpr, expr);
                                                                             suffix_type = lit->cty;
                                                                             literal_type = type_f32;
                                                                             } break;
                                                                             default:
                                                                             interp->report_error(expr->pos(), "Compiler Error: Trying to check a not literal as a literal");
                                                                             return nullptr;
                                                                             }

                                                                             auto cty_type = get_type_from_suffix_type(suffix_type);
                                                                             if(cty_type) {
                                                                             /// or move this to the switch statement above (that is allot of duplicate code though)
                                                                             // check_literal_sizes(expr, cty_type)

                                                                             return cty_type;
                                                                             }
                                                                             return literal_type;
                                                                             }

mist::Type *mist::Typer::get_type_from_suffix_type(ast::ConstantType cty) {
    switch(cty) {
        case ast::ConstantType::None: return nullptr;
        case ast::ConstantType::I8:   return type_i8;
        case ast::ConstantType::I16:  return type_i16;
        case ast::ConstantType::I32:  return type_i32;
        case ast::ConstantType::I64:  return type_i64;
        case ast::ConstantType::U8:   return type_u8;
        case ast::ConstantType::U16:  return type_u16;
        case ast::ConstantType::U32:  return type_u32;
        case ast::ConstantType::U64:  return type_u64;
        case ast::ConstantType::F32:  return type_f32;
        case ast::ConstantType::F64:  return type_f64;
        case ast::ConstantType::Char: return type_char;
    }
}

mist::Type *mist::Typer::resolve_unary_expr(ast::UnaryExpr *expr) {
    auto type = resolve_expr(expr->expr);
    if(!type)
        return nullptr;

    if(type->is_primative()) {
        auto ptype = CAST_PTR(PrimitiveType, type);

        switch(expr->op) {
            case ast::UMinus: {
                // for all types this is the same type
                return type;
            }
            case ast::UBang: {
                if(ptype->is_integer() || ptype->is_boolean())
                    return type_bool;
                else {
                    interp->report_error(expr->pos(), "negation operator can only be applied to integers and booleans: type found: '%s'", ptype->to_string().c_str());
                }
                return nullptr;
            }
            case ast::Tilde: {
                if(ptype->is_integer()) {
                    return ptype;
                }
                else {
                    interp->report_error(expr->pos(), "unable to apply bitwise negation to a non-integer expression: type found: '%s'", ptype->to_string().c_str());
                }
                return nullptr;
            }
            case ast::UAmpersand: {
                // pointer type

                if(expr->expr->is_literal()) {
                    interp->report_error(expr->pos(), "unable to take address of literal");
                    return nullptr;
                }

                auto type = new PointerType(ptype);

                // Check if this type already exists
                Type* found_type = interp->find_type(type);

                // if it does then use that type
                if(found_type) {
                    delete type;
                    return found_type;
                }
                else {
                    // it doesnt, so add it and return it.
                    return interp->add_type(type);
                }
            }
            case ast::UAstrick: {
                interp->report_error(expr->pos(), "de-referencing a non pointer type: found type: '%s'", ptype->to_string().c_str());
                return nullptr;
            }
            default:
               break;
        }
    }
    else if(type->is_named_type()) {
        interp->report_error(expr->expr->pos(), "[LOG] Overloading unary operator is not implemented");
    }
    return nullptr;
}

mist::Type *mist::Typer::resolve_binary_expr(ast::BinaryExpr *expr) {
    auto lhsType = resolve_expr(expr->lhs);
    auto rhsType = resolve_expr(expr->rhs);

    if(lhsType->is_named_type()) {
        interp->report_error(expr->lhs->pos(), "operator overloading is not implemented");
        return nullptr;
    }

    if((lhsType->is_primative() && rhsType->is_primative())) {
        // the types must be primatives at this point.

        auto lhsPType = CAST_PTR(PrimitiveType, lhsType);
        auto rhsPType = CAST_PTR(PrimitiveType, rhsType);

        switch(expr->op) {
            case ast::BinaryOp::Plus:
            case ast::BinaryOp::BMinus:
            case ast::BinaryOp::Slash:
            case ast::BinaryOp::BAstrick:
            case ast::BinaryOp::Percent:
            case ast::BinaryOp::AstrickAstrick: {

                if(lhsPType->is_boolean() || rhsPType->is_boolean())
                    break;
                else if(lhsPType->is_character() || rhsPType->is_character())
                    break;

                if(expr->op == ast::BinaryOp::Percent|| expr->op == ast::BinaryOp::AstrickAstrick)
                    std::clog << "[LOG] using % and ** operators these are not fully implemented with every primative type";

                // if they are the same type, then use that type.
                if(lhsType == rhsType)
                    return lhsType;


                if(lhsPType->is_integer() && rhsPType->is_integer())
                    return max_type_size(lhsType, rhsType);

                // if they are both float point values but with different sizes then use the large of the two
                if(lhsPType->is_floating() && rhsPType->is_floating())
                    return max_type_size(lhsType, rhsType);

                // at this point one of them add to float
                // return the float
                if(lhsPType->is_floating())
                    return lhsPType;
                else
                    return rhsPType;
            }
            case ast::BinaryOp::BAmpersand:
            case ast::BinaryOp::LessLess:
            case ast::BinaryOp::GreaterGreater:
            case ast::BinaryOp::Pipe:
            case ast::BinaryOp::Carrot: {
                if(lhsPType->is_integer() && rhsPType->is_integer())
                    return max_type_size(lhsType, rhsType);
                break;
            }

            case ast::BinaryOp::Less:
            case ast::BinaryOp::Greater:
            case ast::BinaryOp::LessEqual:
            case ast::BinaryOp::GreaterEqual: {
                if(lhsPType->is_boolean() || rhsPType->is_boolean())
                    break;
                else if(lhsPType->is_character() || rhsPType->is_character()) {
                    if(lhsPType->is_character()) {
                       if(!rhsPType->is_integer()) {
                           break;
                       }
                    }
                    else {
                        if(!lhsPType->is_integer()) {
                            break;
                        }
                    }
                }

                // if they are primatives the this should will work for any combination of
                return type_bool;
            }

            // all of the primatives are comparable with eachother
            case ast::BinaryOp::EqualEqual:
            case ast::BinaryOp::BangEqual: {
                if (lhsPType->is_boolean() || rhsPType->is_boolean()) {
                    if (lhsPType->is_boolean() && rhsPType->is_boolean())
                        return type_bool;
                    else {
                        break;
                    }
                }
                return type_bool;
            }
        }
    }
    else if(lhsType->is_pointer() || rhsType->is_pointer()) {
        switch(expr->op) {
            case ast::BinaryOp::Plus: {
                if(lhsType->is_pointer()) {
                    if(rhsType->is_primative()) {
                        auto rhsPType = CAST_PTR(PrimitiveType, rhsType);
                        if(rhsPType->is_integer()) {
                            return lhsType;
                        }
                    }
                } else {
                    if(lhsType->is_primative()) {
                        auto lhsPType = CAST_PTR(PrimitiveType, rhsType);
                        if(lhsPType->is_integer()) {
                            return rhsType;
                        }
                    }
                }
                break;
            }
            case ast::BinaryOp::BMinus: {
                if(lhsType->is_pointer()) {
                    if(rhsType->is_primative()) {
                        auto rhsPType = CAST_PTR(PrimitiveType, rhsType);
                        if(rhsPType->is_integer()) {
                            return lhsType;
                        }
                    }
                }
                break;

            }
            default:
                break;
        }
    }
    interp->report_error(expr->pos(), "unable to apply '%s' to operands of type: '%s' and '%s'",
            ast::get_binary_op_string(expr->op).c_str(),
            lhsType->to_string().c_str(),
            rhsType->to_string().c_str());
    return nullptr;
}

mist::Type *mist::Typer::resolve_value(ast::ValueExpr *expr) {
    DeclInfo* info = resolve_name(expr->name);
    if(info->is_variable()) {
        return info->type;
    }
    else
        interp->report_error(expr->pos(), "'%s' does not name a variable", expr->name->value->val);
    return nullptr;
}

mist::Type *mist::Typer::resolve_block(ast::BlockExpr *block) {
    auto type = type_unit;
    push_scope(BlockScope);
    for(auto expr : block->elements)
        type = resolve_expr(expr);
    pop_scope();
    return type;
}

    //
// Created by Andrew Bregger on 2018-12-29.
//

#include "typer.hpp"

#include "frontend/parser/ast/ast_printer.hpp"


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
    return k == Variable;
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

mist::Val mist::Typer::resolve_expr(ast::Expr *expr) {
    if(!expr) {
        std::clog << "[LOG]" << "null expression" << std::endl;
        return Val();
    }
    Val val;
    switch(expr->kind()) {
        case ast::UnitLit:
            return Val(type_unit);
        case ast::StringConst:
            interp->report_error(expr->pos(), "Strings are not implemented at this time");
            return Val();
        case ast::IntegerConst:
        case ast::CharConst:
        case ast::FloatConst:
            val = check_integral_literal_and_type_suffix(expr);
            break;
        case ast::BooleanConst:
            val = Val(type_bool);
            break;
        case ast::Unary:
            val = resolve_unary_expr(CAST_PTR(ast::UnaryExpr, expr));
            break;
        case ast::Binary:
            val = resolve_binary_expr(CAST_PTR(ast::BinaryExpr, expr));
            break;
        case ast::Value:
            val = resolve_value(CAST_PTR(ast::ValueExpr, expr));
            break;
        case ast::Tuple: {
            auto tup = CAST_PTR(ast::TupleExpr, expr);
            std::vector<Type*> elements;
            u64 size = 0;
            for(auto elems : tup->values) {
                auto t = resolve_expr(elems);
                if(!t.type)
                    break;
                elements.push_back(t.type);
                size += t.type->size();
            }
            Type* tupletype = new TupleType(elements, size);
            val.type = interp->find_type(tupletype);
            if(val.type)
                delete tupletype;
            else {
                val.type = interp->add_type(tupletype);
            }
            break;
        }
        case ast::Block:
            val = resolve_block(CAST_PTR(ast::BlockExpr, expr));
            break;
        case ast::DeclDecl: {
            auto decl = CAST_PTR(ast::DeclExpr, expr);
            resolve_decl(decl->decl);
            auto val = Val(type_unit);
            val.expr = expr;
            return val;
        } break;
        case ast::Assignment: {
            val = resolve_assignment_expr(CAST_PTR(ast::AssignmentExpr, expr));
        } break;
        case ast::List: {
            auto e = CAST_PTR(ast::ListExpr, expr);
            std::vector<Type*> subexprs;
            u64 sz = 0;
//            subexprs.reserve(e->subexpr.size());
            for(auto x : e->subexpr) {
                auto st = resolve_expr(x);
                if(st.type) {
                    subexprs.push_back(st.type);
                    sz += st.type->size();
                }
                else break;
            }
            val.type = new TupleType(subexprs, sz);
        } break;
        case ast::Parenthesis:
        case ast::Selector: {
            auto selector = CAST_PTR(ast::SelectorExpr, expr);
            auto operandType = resolve_expr(selector->operand);
            if(!operandType.type) {
                val = Val();
                break;
            }
            if(operandType.type->is_named_type()) {
                if(operandType.type->kind() == Struct) {
                    auto st = CAST_PTR(StructType, operandType.type);
                    auto info = st->info;

                    DeclInfo* selected = resolve_name(info->scope, selector->element->name, true);
                    if(selected) {
                        std::cout << selected->name->value() << std::endl;
                        std::cout << selected->type->to_string() << std::endl;
                        val.type = selected->type;
                        break;
                    }
                    else {
                        interp->report_error(selector->element->pos(),
                                "operand of type: '%s' does not have field: '%s'",
                                st->to_string(tuples_as_lists).c_str(),
                                selector->element->name->value->val.c_str());
                        val.type = nullptr;
                        break;
                    }
                }
            }
            interp->report_error(selector->operand->pos(),
                    "operand of type: '%s' does not contain any fields",
                    operandType.type->to_string(tuples_as_lists).c_str());
            val = Val();
        } break;
        case ast::TupleIndex: {
            auto index = CAST_PTR(ast::TupleIndexExpr, expr);
            auto operandType = resolve_expr(index->operand);

            if(!operandType.type) {
                val = Val();
                break;
            }

            if(operandType.type->is_tuple()) {
                auto tupleType = CAST_PTR(TupleType, operandType.type);
                u32 tupleIndex = tupleType->num();
                u32 indx = index->index;

                if(indx >= tupleIndex) {
                    interp->report_error(index->pos(),
                            "tuple index out of bounds: tuple size '%u' attempted index '%u'",
                            tupleIndex, indx);
                    val = Val();
                    break;
                }

                val.type = tupleType->get(indx);
            }
            else {
                interp->report_error(index->operand->pos(),
                        "attempting to tuple index non tuple type: found type: '%s'",
                        operandType.type->to_string(false).c_str());
                val = Val();
                break;
            }
        } break;
        default:
            break;

    }

    // checks whether the resulting type already exists if
    auto t = interp->find_type(val.type);

    // if one was found and they are not the same.
    if(t && t != val.type) {
        delete val.type;
        val.type = t;
    }
    expr->t = val.type;
    std::cout << "Resolved Expression: " << expr->pos().line << " " << expr->pos().column << ": ";
    if(val.type) {
        std::cout << val.type->to_string(tuples_as_lists);
        std::cout << " Val: " << val;
    }
    else
        std::cout <<  "Failed";
    std::cout << std::endl;
    return val;
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
                    interp->report_error("[LOG]: Cyclic reference, I think");
                }
            }
        } break;
        case ast::TupleType: {
            auto name = CAST_PTR(ast::TupleSpec, spec);
            std::vector<Type*> specs;
            u32 size = 0;
            for(auto spec : name->types) {
                Type* t = resolve_typespec(spec);
                specs.push_back(t);
                size = t->size();
            }
            return new TupleType(specs, size);
        } break;
        case ast::FunctionType: {
            auto fn = CAST_PTR(ast::FunctionSpec, spec);
            TupleType* paramType = nullptr;
            u64 paramSize = 0;
            TupleType* returnType = nullptr;
            u64 returnSize = 0;

            std::vector<Type*> params;
            for(auto p : fn->parameters) {
                auto ty = resolve_typespec(p);
                if(p) {
                    params.push_back(ty);
                    paramSize += ty->size();
                }
                else
                    return nullptr;
            }

            std::vector<Type*> returns;
            for(auto p : fn->returns) {
                auto ty = resolve_typespec(p);
                if(p) {
                    returns.push_back(ty);
                    returnSize += ty->size();
                }
                else
                    return nullptr;
            }

            if(returns.empty()) {
                returns.push_back(type_unit);
            }

            paramType = new TupleType(params, paramSize);
            returnType = new TupleType(returns, returnSize);

            type = new FunctionType(paramType, returnType);
        } break;
        case ast::TypeClassType: {
            auto name = CAST_PTR(ast::TypeClassSpec, spec);

        } break;
        case ast::Array: {
            auto name = CAST_PTR(ast::ArraySpec, spec);

            Type* base = resolve_typespec(name->base);

            auto val = resolve_expr(name->size);

            if(val.is_constant) {
                if(val.type->is_primative()) {
                    auto prim = CAST_PTR(PrimitiveType, val.type);
                    if(!prim->is_integer()) {
                        interp->report_error(name->size->pos(), "Array size must be an integer type, found: '%s'", prim->to_string(false).c_str());
                        return nullptr;
                    }

                    switch(prim->pkind()) {
                        case U64:
                            type = new ArrayType(base, val._U64);
                            break;
                        case I64:
                            type = new ArrayType(base, (u64) val._I64);
                            break;
                        default:
                            // if it is any other integer type, the u32 is still valid.
                            type = new ArrayType(base, (u64) val._U32);
                    }

                }
            }
            else {
                // @TODO: Later add support for run time array length.
                //      This would require a rework of the array type.
                //
                // foo :: (n: i32) {
                //      bar: [n]f32;
                // }
                //
                interp->report_error(name->size->pos(), "Array size must be a constant");
                return nullptr;
            }
        } break;
//        case ast::DynamicArray: {
//            auto name = CAST_PTR(ast::DySpec, spec);
//
//        }
        case ast::Map: {
            auto name = CAST_PTR(ast::MapSpec, spec);
        } break;
        case ast::Pointer: {
            auto ptr = CAST_PTR(ast::PointerSpec, spec);
            auto base = resolve_typespec(ptr->base);
            type = new PointerType(base);
        } break;
        case ast::Reference: {
            auto ref = CAST_PTR(ast::ReferenceSpec, spec);
            auto base = resolve_typespec(ref->base);
            type = new ReferenceType(base);
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
        }
        case ast::Unit: {
            return type_unit;
        }
    }
    auto ntype = interp->find_type(type);

    if(ntype && ntype != type) {
        delete type;
        type = ntype;
    }
    else {
        interp->add_type(type);
    }
    return type;
}

mist::Val mist::Typer::resolve_assignment_expr(ast::AssignmentExpr *assign) {



    // always returns unit returns null if failed
    return Val(type_unit);
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

//mist::DeclInfo* mist::Typer::resolve_name_to_declinfo(ast::Expr *name) {
//    switch(name->kind()) {
//        case ast::Value: {
//            auto e = CAST_PTR(ast::ValueExpr, name);
//            return resolve_name(e->name);
//        } break;
//        case ast::Selector: {
//
//        } break;
//    }
//}

mist::Val mist::Typer::resolve_constant_expr(ast::Expr *expr) {
    return Val();
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

    Val type, init_type;

    if(typespec)
        type.type = resolve_typespec(typespec);

    if(init)
        init_type = resolve_expr(init);

    if(!init_type.type && !type.type) {
       // this should have been caught when parsing.
       interp->report_error(decl->pos, "declaration must be given a type or initialization expression,"
                                       "this declaration has neither");

       info->state = Invalid;
       return nullptr;
    }


    Val res;
    // if both are given, then check if they are compatable, otherwise
    if(init_type.type && type.type) {
#ifdef DEBUG
            std::cout << "\tinit_type: " << init_type.type->to_string() << std::endl
                      << "\ttype: " << type.type->to_string() << std::endl;
#endif

        if(equivalent_type(init_type.type, type.type)) {
            res = init_type;
        }
        else {
            interp->report_error(init->pos(), "incompatable types: found '%s', expecting: '%s'", init_type.type->to_string(tuples_as_lists).c_str(), type.type->to_string(tuples_as_lists).c_str());
            return nullptr;
        }
    }
    else if(type.type)
        res = type;
    else if(init_type.type)
        res = init_type;

    Addressing addr = addressing_from_type(res.type);
    info->resolve(res.type, addr, Variable);

#ifdef DEBUG
    std::cout << "Result Type; " << res.type->to_string(tuples_as_lists) << " addressing: "  << addr << std::endl;
#endif

    info->type = res.type;
//    decl->t = res;

    return res.type;
}


/// Assumes the name has already been added to the namespace
mist::Type *mist::Typer::resolve_struct_decl(ast::StructDecl *decl, mist::DeclInfo *info) {

    // this resolves the name of the decl to make sure it can be
    // declared. It also adds it to the scope.

    std::unordered_map<struct String*, Type*> members;
    std::vector<DeclInfo*> infos;


    // creates a new scope for the structure.
    push_scope(MemberScope);
    info->scope = current;

    u64 sz = 0;
    for(auto field : decl->fields) {
        // gets the name of the field
        struct String* name = CAST_PTR(ast::IdentPat, field->name)->name->value;


        auto info = resolve_local_decl(field);

        if(!info)
            break;

        infos.push_back(info);
        members.emplace(name, info->type);

        sz += info->type->size();
    }
    pop_scope();

    // the offsets need to be figured out at here i guess.
    Type* type = new StructType(info->name,  members, info, sz);

    info->resolve(type, Value, Ty);
    return type;
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
        params.push_back(pinfo->type);
        paramsSize += pinfo->type->size();
    }

    for(auto ret : decl->returns) {
        auto type = resolve_typespec(ret);
        returns.push_back(type);
        returnsSize += type->size();
    }

    if(decl->returns.empty()) {
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


    std::cout << "Params: " << paramType->to_string() << std::endl;
    std::cout << "Returns: " << returnType->to_string() << std::endl;

    ast::Expr* returnExpr = nullptr;

    if(decl->body->kind() != ast::Block) {
        push_scope(BlockScope);
        returnExpr = decl->body;
    }
    else {
        auto block = CAST_PTR(ast::BlockExpr, decl->body);
        // there is always a unit expression
        returnExpr = block->elements.back();
    }



    auto body_result = resolve_expr(decl->body);

    std::cout << "Body Result: " << body_result.type->to_string() << std::endl;



    if(decl->body->kind() != ast::Block)
        pop_scope();

    pop_scope();

    // if there are only 1 element in the tuple, we know this
    // isnt an actual tuple.
    if(returnType->num() == 1) {
        check_types(returnType->get(0), body_result.type, returnExpr->pos());
    }
    else {
        bool old = tuples_as_lists;
        tuples_as_lists = true;
        check_types(returnType, body_result.type, returnExpr->pos());
        tuples_as_lists = old;
    }

    auto found_type = interp->find_type(functionType);
    if(found_type && functionType != found_type) {
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
        if(type.type)
            initTypes.push_back(type.type);
        else
            return nullptr;
    }

    std::vector<Type*> annoTypes;
    for(auto spec : decl->sp) {
        auto type = resolve_typespec(spec);
        std::cout << "Resolved type: " << type->to_string(false) << std::endl;
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
                   interp->report_error(decl->sp.front()->pos(), "expecting type: '%s', found type: '%s'", expected->to_string(tuples_as_lists).c_str(), init->to_string(tuples_as_lists).c_str());
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
           case Mutable: {
               auto p1 = CAST_PTR(MutableType, type1);
               auto p2 = CAST_PTR(MutableType, type2);
               return equivalent_type(p1->base(), p2->base());
           }
           case List: {
               auto p1 = CAST_PTR(ListType, type1);
               auto p2 = CAST_PTR(ListType, type2);

               if(p1->num() != p2->num())
                   return false;

               for(u32 i = 0; i < p1->num(); ++i) {
                   if(!equivalent_type(p1->get(i), p2->get(i)))
                       return false;
               }

               return true;
           }
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

mist::Val mist::Typer::check_integral_literal_and_type_suffix(ast::Expr *expr) {
    Type* literal_type = nullptr;
    auto suffix_type = ast::ConstantType::None;
    Val val;
    // we are creates contants from literals, they can not be negative.
    i64 integer = -1;
    f64 floating = -1;
    char character;

    switch(expr->kind()) {
        case ast::IntegerConst: {
            auto lit = CAST_PTR(ast::IntegerConstExpr, expr);
            suffix_type = lit->cty;
            /// the default type for literals integer is i32
            literal_type = type_i32;
            integer = lit->value;
        } break;
        case ast::CharConst: {
            /// I thought i added the a type suffix to character literals. - Andrew B
            auto lit = CAST_PTR(ast::CharConstExpr, expr);
            //            suffix_type = lit->
//            literal_type = type_char;
            character = lit->value;
            return Val(character);
        } break;
        case ast::FloatConst: {
            auto lit = CAST_PTR(ast::FloatConstExpr, expr);
            suffix_type = lit->cty;
            literal_type = type_f32;
            floating = lit->value;
        } break;
        default:
            interp->report_error(expr->pos(), "Compiler Error: Trying to check a not literal as a literal");
            return Val();
    }

    auto cty_type = get_type_from_suffix_type(suffix_type);
    if(cty_type) {
    /// or move this to the switch statement above (that is allot of duplicate code though)
    // check_literal_sizes(expr, cty_type)
       literal_type = cty_type;
    }

    return create_value(literal_type, integer, floating);
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

mist::Val mist::Typer::resolve_unary_expr(ast::UnaryExpr *expr) {
    auto val = resolve_expr(expr->expr);
    auto type = val.type;

    if(!type)
        return Val();

    if(type->is_primative()) {
        auto ptype = CAST_PTR(PrimitiveType, type);

        switch(expr->op) {
            case ast::UMinus: {
                // for all types this is the same type
                auto v = perform_unary_expr(expr->op, val, val.type);
                v.expr = expr;
                return v;
            }
            case ast::UBang: {
                if(ptype->is_integer() || ptype->is_boolean()) {
                    auto v = perform_unary_expr(expr->op, val, val.type);
                    v.expr = expr;
                    return v;
                }
                else {
                    interp->report_error(expr->pos(), "negation operator can only be applied to integers and booleans: type found: '%s'", ptype->to_string(tuples_as_lists).c_str());
                }
                return Val();
            }
            case ast::Tilde: {
                if(ptype->is_integer()) {
                    auto v = perform_unary_expr(expr->op, val, val.type);
                    v.expr = expr;
                    return v;
                }
                else {
                    interp->report_error(expr->pos(), "unable to apply bitwise negation to a non-integer expression: type found: '%s'", ptype->to_string(tuples_as_lists).c_str());
                }
                return Val();
            }
            case ast::UAmpersand: {
                // pointer type

                if(expr->expr->is_literal()) {
                    interp->report_error(expr->pos(), "unable to take address of literal");
                    return Val();
                }

                auto type = new PointerType(ptype);

                // Check if this type already exists
                Type* found_type = interp->find_type(type);

                // if it does then use that type
                if(found_type && found_type != type) {
                    delete type;
                    auto v = Val(found_type);
                    v.expr = expr;
                    return v;
                }
                else {
                    // it doesnt, so add it and return it.
                    Val v = Val(interp->add_type(type));
                    v.expr = expr;
                    return v;
                }
            }
            case ast::UAstrick: {
                interp->report_error(expr->pos(), "de-referencing a non pointer type: found type: '%s'", ptype->to_string(tuples_as_lists).c_str());
                return Val();
            }
            default:
               break;
        }
    }
    else if(type->is_named_type()) {
        interp->report_error(expr->expr->pos(), "[LOG] Overloading unary operator is not implemented");
    }
    return Val();
}

mist::Val mist::Typer::resolve_binary_expr(ast::BinaryExpr *expr) {
    auto lhs = resolve_expr(expr->lhs);
    auto rhs = resolve_expr(expr->rhs);
    auto lhsType = lhs.type;
    auto rhsType = rhs.type;

    if(lhsType->is_named_type()) {
        interp->report_error(expr->lhs->pos(), "operator overloading is not implemented");
        return Val();
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
                Type* type = nullptr;
                if(lhsPType->is_boolean() || rhsPType->is_boolean())
                    break;
                else if(lhsPType->is_character() || rhsPType->is_character())
                    break;

                if(expr->op == ast::BinaryOp::Percent|| expr->op == ast::BinaryOp::AstrickAstrick)
                    std::clog << "[LOG] using % and ** operators these are not fully implemented with every primative type";

                // if they are the same type, then use that type.
                if(lhsType == rhsType)
                     type = lhsType;


                if(lhsPType->is_integer() && rhsPType->is_integer())
                    type = max_type_size(lhsType, rhsType);

                // if they are both float point values but with different sizes then use the large of the two
                if(lhsPType->is_floating() && rhsPType->is_floating())
                    type = max_type_size(lhsType, rhsType);

                // at this point one of them add to float
                // return the float
                if(lhsPType->is_floating())
                    type = lhsPType;
                else
                    type = rhsPType;
                auto val = perform_binary_expr(expr->op, lhs, rhs, type);
                val.expr = expr;
                return val;
            }
            case ast::BinaryOp::BAmpersand:
            case ast::BinaryOp::LessLess:
            case ast::BinaryOp::GreaterGreater:
            case ast::BinaryOp::Pipe:
            case ast::BinaryOp::Carrot: {
                if(lhsPType->is_integer() && rhsPType->is_integer()) {
                    auto expected = max_type_size(lhsType, rhsType);
                    Val v = perform_binary_expr(expr->op, lhs, rhs, expected);
                    v.expr = expr;
                    return v;
                }
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
                Val v = perform_binary_expr(expr->op, lhs, rhs, type_bool);
                v.expr = expr;
                return v;
            }

            // all of the primatives are comparable with eachother
            case ast::BinaryOp::EqualEqual:
            case ast::BinaryOp::BangEqual: {
                if (lhsPType->is_boolean() || rhsPType->is_boolean()) {
                    if (lhsPType->is_boolean() && rhsPType->is_boolean()) {
                        Val v = Val(type_bool);
                        v.expr = expr;
                        return v;
                    }
                    else {
                        break;
                    }
                }
                Val v = Val(type_bool);
                v.expr = expr;
                return v;
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
                            auto v = Val(lhsType);
                            v.expr = expr;
                            return v;
                        }
                    }
                } else {
                    if(lhsType->is_primative()) {
                        auto lhsPType = CAST_PTR(PrimitiveType, rhsType);
                        if(lhsPType->is_integer()) {
                            auto v = Val(rhsType);
                            v.expr = expr;
                            return v;
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
                            Val v = Val(lhsType);
                            v.expr = expr;
                            return v;
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
            lhsType->to_string(tuples_as_lists).c_str(),
            rhsType->to_string(tuples_as_lists).c_str());
    return Val();
}

mist::Val mist::Typer::resolve_value(ast::ValueExpr *expr) {
    DeclInfo* info = resolve_name(expr->name);
    if(!info) {
        return Val();
    }
    if(info->is_variable() || info->is_function()) {
        Val val = Val(info->type);
        val.expr = expr;
        return val;
    }
    else
        interp->report_error(expr->pos(), "'%s' does not name a variable or function", expr->name->value->val.c_str());
    return Val();
}

mist::Val mist::Typer::resolve_block(ast::BlockExpr *block) {
    Val val;
    push_scope(BlockScope);
    for(auto expr : block->elements)
        val = resolve_expr(expr);
    pop_scope();
    val.expr = block;
    return val;
}

void mist::Typer::check_types(mist::Type *t1, mist::Type *t2, Pos pos) {
    if(!equivalent_type(t1, t2)) {
        interp->report_error(pos, "expecting type: '%s', found: '%s'", t1->to_string(tuples_as_lists).c_str(),
                t2->to_string(tuples_as_lists).c_str());
    }
}

mist::Val mist::Typer::create_value(mist::Type *type, i64 i, f64 f) {
    if(!type)
        return Val();

    if(type->is_primative()) {
        auto prim = CAST_PTR(PrimitiveType, type);
        switch(prim->pkind()) {
            case I8:
                if(i != -1)
                    return Val((i8) i);
                else
                    return Val((i8) f);
            case I16:
                if(i != -1)
                    return Val((i16)i);
                else
                    return Val((i16) f);
            case I32:
                if(i != -1)
                    return Val((i32)i);
                else
                    return Val((i32) f);
            case I64:
                if(i != -1)
                    return Val((i64)i);
                else
                    return Val((i64) f);
            case U8:
                if(i != -1)
                    return Val((u8)i);
                else
                    return Val((u8) f);
            case U16:
                if(i != -1)
                    return Val((u16)i);
                else
                    return Val((u16) f);
            case U32:
                if(i != -1)
                    return Val((u32)i);
                else
                    return Val((u32) f);
            case U64:
                if(i != -1)
                    return Val((u64)i);
                else
                    return Val((u64) f);
            case F32:
                if(i != -1)
                    return Val((f32)i);
                else
                    return Val((f32) f);
            case F64:
                if(i != -1)
                    return Val((f64)i);
                else
                    return Val((f64) f);
            case Char:
                if(i != -1)
                    return Val((char) i);
            case Bool:
                if(i != -1)
                    return Val(i != 0);
                else
                    return Val(f != 0.0);
        }
    }
    else {
        return Val(type);
    }
}

mist::Val mist::Typer::cast_value(mist::Val val, mist::Type *type) {

#define CAST_VAL(ty, t, val) {\
    PrimitiveType* pType = CAST_PTR(PrimitiveType, val.type); \
    Val nval; \
    switch(pType->pkind()) { \
        case I8: \
        case I16: \
        case I32: \
            nval = Val((ty) val._I32); \
            break; \
        case I64: \
            nval = Val((ty) val._I64); \
            break; \
        case U8: \
        case U16: \
        case U32: \
            nval = Val((ty) val._U32); \
            break; \
        case U64: \
            nval = Val((ty) val._U64); \
            break; \
        case F32: \
            nval = Val((ty) val._F32); \
            break; \
        case F64: \
            nval = Val((ty) val._F64); \
            break; \
        case Char: \
            nval = Val((ty) val._Char); \
            break; \
        case Bool: \
            nval = Val((ty) val._Bool); \
            break; \
    } \
    nval.type = t; \
    nval.expr = val.expr; \
    return nval; \
}

    if(val.type == nullptr) {
        return Val(type);
    }
    else {
        if(val.is_constant) {
            if(val.type->is_primative()) {
                if(type->is_primative()) {
                    PrimitiveType* nType = CAST_PTR(PrimitiveType, type);
                    switch(nType->pkind()) {
                        case I8:
                            CAST_VAL(i8, nType, val);
                        case I16:
                            CAST_VAL(i16, nType, val);
                        case I32:
                            CAST_VAL(i32, nType, val);
                        case I64:
                            CAST_VAL(i64, nType, val);
                        case U8:
                            CAST_VAL(u8, nType, val);
                        case U16:
                            CAST_VAL(u16, nType, val);
                        case U32:
                            CAST_VAL(u32, nType, val);
                        case U64:
                            CAST_VAL(u64, nType, val);
                        case F32:
                            CAST_VAL(f32, nType, val);
                        case F64:
                            CAST_VAL(f64, nType, val);
                        case Char:
                            CAST_VAL(char, nType, val);
                        case Bool:
                            CAST_VAL(bool, nType, val);
                    }
                    interp->report_error(val.expr->pos(), "Failed to cast value");
                    return Val();
                }
            }
            else {
                interp->report_error(val.expr->pos(), "Attempting to cast a non primative constant");
            }
        }
        else
            return Val(type);
    }
#undef CAST_VAL
    return mist::Val();
}

#define UOPERATOR(op, v, t) { \
    PrimitiveType* pType = CAST_PTR(PrimitiveType, t); \
    Val nval; \
    switch(pType->pkind()) { \
        case I8: \
            nval = Val(op v._I8); \
            break; \
        case I16: \
            nval = Val(op v._I16); \
            break; \
        case I32: \
            nval = Val(op v._I32); \
            break; \
        case I64: \
            nval = Val(op v._I64); \
            break; \
        case U8: \
            nval = Val(op v._U8); \
            break; \
        case U16: \
            nval = Val(op v._U16); \
            break; \
        case U32: \
            nval = Val(op v._U32); \
            break; \
        case U64: \
            nval = Val(op v._U64); \
            break; \
        case F32: \
            nval = Val(op v._F32); \
            break; \
        case F64: \
            nval = Val(op v._F64); \
            break; \
        case Char: \
            nval = Val(op v._Char); \
            break; \
        case Bool: \
            break; \
    } \
    val = nval; \
    val.type = t; \
}

#define UOPERATOR_WITHOUTFLOAT(op, v, t) { \
    PrimitiveType* pType = CAST_PTR(PrimitiveType, t); \
    Val nval; \
    switch(pType->pkind()) { \
        case I8: \
            nval = Val(op v._I8); \
            break; \
        case I16: \
            nval = Val(op v._I16); \
            break; \
        case I32: \
            nval = Val(op v._I32); \
            break; \
        case I64: \
            nval = Val(op v._I64); \
            break; \
        case U8: \
            nval = Val(op v._U8); \
            break; \
        case U16: \
            nval = Val(op v._U16); \
            break; \
        case U32: \
            nval = Val(op v._U32); \
            break; \
        case U64: \
            nval = Val(op v._U64); \
            break; \
        case Char: \
            nval = Val(op v._Char); \
            break; \
        case Bool: \
            break; \
    } \
    val = nval; \
    nval.type = t; \
}

#define BOPERATOR(op, lhs, rhs, t) { \
    PrimitiveType* pType = CAST_PTR(PrimitiveType, t); \
    Val nval; \
    switch(pType->pkind()) { \
        case I8: \
            nval = Val(lhs._I8 op rhs._I8); \
            break; \
        case I16: \
            nval = Val(lhs._I16 op rhs._I16); \
            break; \
        case I32: \
            nval = Val(lhs._I32 op rhs._I32); \
            break; \
        case I64: \
            nval = Val(lhs._I64 op rhs._I64); \
            break; \
        case U8: \
            nval = Val(lhs._U8 op rhs._U8); \
            break; \
        case U16: \
            nval = Val(lhs._U16 op rhs._U16); \
            break; \
        case U32: \
            nval = Val(lhs._U32 op rhs._U32); \
            break; \
        case U64: \
            nval = Val(lhs._U64 op rhs._U64); \
            break; \
        case F32: \
            nval = Val(lhs._F32 op rhs._F32); \
            break; \
        case F64: \
            nval = Val(lhs._F64 op rhs._F64); \
            break; \
        case Char: \
            nval = Val(lhs._Char op rhs._Char); \
            break; \
        case Bool: \
            break; \
    } \
    val = nval; \
    val.type = t; \
}

#define BOPERATOR_WITHOUTFLOAT(op, lhs, rhs, t) { \
    PrimitiveType* pType = CAST_PTR(PrimitiveType, t); \
    Val nval; \
    switch(pType->pkind()) { \
        case I8: \
            nval = Val(lhs._I8 op rhs._I8); \
            break; \
        case I16: \
            nval = Val(lhs._I16 op rhs._I16); \
            break; \
        case I32: \
            nval = Val(lhs._I32 op rhs._I32); \
            break; \
        case I64: \
            nval = Val(lhs._I64 op rhs._I64); \
            break; \
        case U8: \
            nval = Val(lhs._U8 op rhs._U8); \
            break; \
        case U16: \
            nval = Val(lhs._U16 op rhs._U16); \
            break; \
        case U32: \
            nval = Val(lhs._U32 op rhs._U32); \
            break; \
        case U64: \
            nval = Val(lhs._U64 op rhs._U64); \
            break; \
        case Char: \
            nval = Val(lhs._Char op rhs._Char); \
            break; \
        case Bool: \
            break; \
    } \
    nval.type = t; \
    val = nval; \
}

mist::Val mist::Typer::perform_binary_expr(ast::BinaryOp op, mist::Val lhs, mist::Val rhs, mist::Type *expected_type) {
    if(!lhs.is_constant && !rhs.is_constant)
        return Val(expected_type);

    Val val;

    Val castLhs = cast_value(lhs, expected_type);
    Val castRhs = cast_value(rhs, expected_type);

    switch(op) {
        case ast::Plus:
            BOPERATOR(+, castLhs, castRhs, expected_type)
            break;
        case ast::BMinus:
            BOPERATOR(-, castLhs, castRhs, expected_type)
            break;
        case ast::Slash:
            BOPERATOR(/, castLhs, castRhs, expected_type)
            break;
        case ast::Percent:
            BOPERATOR_WITHOUTFLOAT(%, castLhs, castRhs, expected_type)
            break;
        case ast::BAstrick:
            BOPERATOR(*, castLhs, castRhs, expected_type)
            break;
        case ast::AstrickAstrick:
//            BOPERATOR(, castLhs, castRhs, expected_type)
            interp->report_error(lhs.expr->pos(), "constant '**' operator is not implemented at this time");
            break;
        case ast::BAmpersand:

            BOPERATOR_WITHOUTFLOAT(&, castLhs, castRhs, expected_type)
            break;
        case ast::LessLess:
            BOPERATOR_WITHOUTFLOAT(<<, castLhs, castRhs, expected_type)
            break;
        case ast::GreaterGreater:
            BOPERATOR_WITHOUTFLOAT(>>, castLhs, castRhs, expected_type)
            break;
        case ast::Pipe:
            BOPERATOR_WITHOUTFLOAT(|, castLhs, castRhs, expected_type)
            break;
        case ast::Carrot:
            BOPERATOR_WITHOUTFLOAT(^, castLhs, castRhs, expected_type)
            break;
        case ast::Less:
            BOPERATOR(<, castLhs, castRhs, expected_type)
            break;
        case ast::Greater:
            BOPERATOR(>, castLhs, castRhs, expected_type)
            break;
        case ast::LessEqual:
            BOPERATOR(<=, castLhs, castRhs, expected_type)
            break;
        case ast::GreaterEqual:
            BOPERATOR(>=, castLhs, castRhs, expected_type)
            break;
        case ast::EqualEqual:
            BOPERATOR(==, castLhs, castRhs, expected_type)
            break;
        case ast::BangEqual:
            BOPERATOR(!=, castLhs, castRhs, expected_type)
            break;
    }
    return val;
}

mist::Val mist::Typer::perform_unary_expr(ast::UnaryOp op, mist::Val v, mist::Type *expected_type) {
    if(!v.is_constant)
        return Val(expected_type);

    Val val;

    Val castVal = cast_value(v, expected_type);

    switch(op) {
        case ast::UMinus:
            UOPERATOR(-, castVal, expected_type)
            break;
        case ast::UBang:
            UOPERATOR(!, castVal, expected_type)
            break;
        case ast::Tilde:
            UOPERATOR_WITHOUTFLOAT(~, castVal, expected_type)
            break;
        case ast::UAmpersand:
        case ast::UAstrick:
            interp->report_error(v.expr->pos(), "Compiler Error: Attempting to evaluator '&' or '*' on a constant value");
            return Val(expected_type);
    }

    return val;
}


//
// Created by Andrew Bregger on 2018-12-29.
//

#pragma once

#include <tuple>

#include "scope.hpp"

#include "frontend/checker/value.hpp"
#include "frontend/parser/ast/ast.hpp"
#include "frontend/parser/ast/ast_common.hpp"
#include "frontend/parser/ast/ast_expr.hpp"
#include "frontend/parser/ast/ast_decl.hpp"
#include "frontend/parser/ast/ast_typespec.hpp"
#include "frontend/parser/ast/ast_pattern.hpp"

#include "types/type.hpp"
#include "frontend/checker/value.hpp"

extern mist::Type* type_u8;
extern mist::Type* type_u16;
extern mist::Type* type_u32;
extern mist::Type* type_u64;

extern mist::Type* type_i8;
extern mist::Type* type_i16;
extern mist::Type* type_i32;
extern mist::Type* type_i64;

extern mist::Type* type_f32;
extern mist::Type* type_f64;

extern mist::Type* type_char;
extern mist::Type* type_bool;
extern mist::Type* type_unit;
extern mist::Type* type_emptytuple;

namespace mist {


    enum DeclInfoKind {
        Unknown, // default
        Variable,
        Func,
        Ty,
        Mod,
    };

    enum Addressing {
        Value,
        Address
    };

    enum DeclInfoState {
        Unresovled,
        Resolved,
        Resolving,
        Invalid /// there was an error when resolving the decl
    };

    struct DeclInfo {
        DeclInfoState state;
        struct mist::String* name;      /// the name of this entity
        ast::Decl* decl;                /// the declaration this name is derived from
        i32 decl_index;                 /// the index of this name with in the declaration
                                        /// This index is -1 for declaration which only have a single name
                                        /// this is used to index into the pattern structure of local declarations.
         Type* type{nullptr};
         Scope* scope{nullptr};

         DeclInfoKind k;
         Addressing addressing{Value}; /// This is only used for locals
         std::vector<mist::FunctionType*> overloads; /// Maybe this is how function can be overloaded

         DeclInfo(struct String *name, i32 index, mist::Scope *scope, ast::Decl *decl);

         bool is_type();

         bool is_local();

         bool is_variable();

         bool is_function();

         bool is_struct();

         bool is_class();

         bool is_typeclass();

         bool is_resolved();

         bool resolving();


         void resolve(Type* type, Addressing addr, DeclInfoKind k);
    };

    class Typer {
    public:
        Typer(Interpreter* interp);

        void create_root();

        std::vector<DeclInfo *> add_pattern(ast::Pattern *pattern, ast::Decl *decl, i32 index = -1);

        bool add_name(ast::Ident* name, ast::Decl* decl);

        bool add_name(ast::Ident *name, DeclInfo *info);

        /// Resolves a declaration from the information called
        // @param info the information about the declarations and the declaration node itself.
        /// @note: Called explicitly for toplevel declarations while the previous
        ///        function is used for all other declarations. I.E. handles the out
        ///        of order declarations.
        /// @return the type of the declaration
//        Type* resolve_decl(DeclInfo* info);

        /// Resolves an expression to its type
        /// @return the type of the declaration
        Val resolve_expr(ast::Expr* expr);

        /// Resolves an expression to its type
        /// @return the type of the declaration
        Val  resolve_constant_expr(ast::Expr* expr);

        /// Resolves integral literal (char, integers, and floats)
        /// if a type suffix follows, check the type
        /// @note: transforms the literal node if the base type is changed i.g. integer -> float 1f32
        /// @return the type of the literal
        Val check_integral_literal_and_type_suffix(ast::Expr *expr);

        /// @return the type of the declaration
        Type* resolve_typespec(ast::TypeSpec* spec);

        /// Check if two types are equivalent.
        /// @param type1 the first type
        /// @param type2 the second type
        /// @ param ignore_mut if true then if one of the types is wrapped in mut then it is ignored
        bool equivalent_type(mist::Type *type1, mist::Type *type2, bool ignore_mut = false);

        /// Resolves unary expression
        /// @param expr expression
        /// @return the type of the expression
        Val resolve_unary_expr(ast::UnaryExpr* expr);

        /// Performs a binary operation on a constant val
        /// @param op the unary operator to apply to val
        /// @param val the value being operated on.
        /// @param expected_type the type expected following the operation
        /// @return the transformed type.
        Val perform_binary_expr(ast::BinaryOp op, Val lhs, Val rhs, Type* expected_type);

        /// Resolves binary expression
        /// @param expr expression
        /// @return the type of the expression
        Val resolve_binary_expr(ast::BinaryExpr* expr);

        /// Performs a unary operation on a constant val
        /// @param op the binary operator to apply to val
        /// @param lhs the value being operated on the lhs
        /// @param rhs the value being operated on the rhs
        /// @param expected_type the type expected following the operation
        /// @return the transformed type.
        Val perform_unary_expr(ast::UnaryOp op, Val val, Type* expected_type);

        Val resolve_value(ast::ValueExpr* expr);

        /// Resolves a block expression to the resulting expression
        /// @param block the block containing the list of expressions
        ///              to resolve
        Val resolve_block(ast::BlockExpr* block);

        /// Resolves an assignment expression
        /// @param assign the assignment expression with variables and expressions
        /// @return unit
        Val resolve_assignment_expr(ast::AssignmentExpr* assign);

        /// Resolves an expression to a Declaration Info
        ///     returns null if it isn't a valid expression
        /// @param name the epression to resolve
        /// @return the delcaration information or null
        DeclInfo* resolve_name_to_declinfo(ast::Expr* name);

        /// Resolves the name to the declaration information.
        /// @param name the name of the declaration
        /// @return the declaration infornation of the name.
        DeclInfo* resolve_name(ast::Ident* name);

        /// Resolves the name in a specificed scope
        /// @param name the name of the declaration
        /// @param scope the scope to be search
        /// @return the declaration infornation of the name.
        DeclInfo* resolve_name(Scope* scope, ast::Ident* name, bool only_scope = true);


        /// Resolves local declarations
        /// @param local the local declaration to be resolved
        std::vector<DeclInfo*> resolve_local(ast::LocalDecl* local);

        /// resolves all of the declarations contained in the module
        /// @param module the module which contains all of the declarations
        void resolve_module(ast::Module* module);

        Type* resolve_toplevel_decl(DeclInfo* info);

        Type* resolve_global_decl(ast::GlobalDecl* decl, DeclInfo* info);

        Type* resolve_struct_decl(ast::StructDecl* decl, DeclInfo* info);

        Type* resolve_class_decl(ast::ClassDecl* decl, DeclInfo* info);

        Type* resolve_function_decl(ast::FunctionDecl *decl, DeclInfo *info);


        void resolve_decl(ast::Decl* decl);

//        /// @param decl the implelementation block declaration
//        /// @param info the declaration info
/// Tthis still needs to be thought about
        Type* resolve_impl_decl(ast::ImplDecl* decl, DeclInfo* info);

        /// Resolving a local decl can produce multiple declinfos
        /// @param decl the local declaration node
        /// @return this function returns the first declaration info
        ///         this is used by resolving functions and structure/classes
        DeclInfo * resolve_local_decl(ast::LocalDecl *decl);

        Scope * push_scope(ScopeKind kind);

        Type * add_prelude(struct mist::String *name, Type *type, Addressing addr, DeclInfoKind k);

        void pop_scope();

        inline Scope* get_current() { return current; }

        inline Type* max_type_size(Type* t1, Type* t2) { return (t1->size() > t2->size() ? t1 : t2); }

    private: // helper functions

        Type* get_type_from_suffix_type(ast::ConstantType cty);

        Addressing addressing_from_type(Type* type);

        void check_types(Type* t1, Type* t2, Pos pos);

        /// Creates the appropriate type val from the type.
        /// @param type the type of the value
        /// @param i an integer value for the value to be created from
        /// @param f a float value for the value to be created from
        /// @return value
        Val create_value(Type* type, i64 i = -1, f64 f = -1);

        /// Casts a value to a new type. It will convert the constant if
        /// it exists.
        /// @param val the original value
        /// @param type the new type
        /// @return the casted value
        Val cast_value(Val val, Type* type);

    private: // internal state

        Interpreter* interp{nullptr};

        Scope* root; // scope of the root of the currentmodule
        Scope* prelude; // the prelude scope, the scope of the built in functions and types.
        Scope* current;


        // local state for to help printing error messages.
        bool tuples_as_lists{false};
    };
}

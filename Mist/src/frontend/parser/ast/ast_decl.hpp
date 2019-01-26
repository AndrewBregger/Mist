#pragma once

#include "ast_common.hpp"
namespace mist {
	class Scope;
}

namespace ast {

	struct Type;
	struct Expr;
	struct TypeSpec;
	struct Pattern;


	enum DeclKind {
		Global,
		Local,
		SelfField,
//		MultiLocal,
		Struct,
		Class,
		TypeClass,
		Function,
		OpFunction,
		Use, // module system
		Impl,
		Generic,
		Variant, // rename to variant
		VariantMember,
		Alias, // type alias (obviously)
		Prelude
	};

	enum Op {
		OpPlus,
		OpMinus,
		OpSlash,
		OpPercent,
		OpAstrick,
		OpAstrickAstrick,
		OpLessLess,
		OpGreaterGreater,
		OpAmpersand,
		OpPipe,
		OpCarrot,
		OpTilde,
		OpBang,
		OpLess,
		OpGreater,
		OpLessEqual,
		OpGreaterEqual,
		OpEqualEqual,
		OpBangEqual,
		OpParenthesis
	};

	struct Decl {
		Visibility vis{Private};
		DeclKind k;
		Ident* name{nullptr};
		Type* t{nullptr};
		mist::Pos pos;

		Decl(Ident* name, DeclKind k, mist::Pos pos);

		DeclKind kind();

		Type* type();

		mist::Scope* get_scope();

		const std::string& string();
	};

	struct GenericDecl :  public Decl {
		std::vector<TypeSpec*> bounds;

		GenericDecl(Ident* name, const std::vector<TypeSpec*>& bounds, mist::Pos pos);
	};

	struct Generics {
		std::vector<GenericDecl*> parameters;

		Generics(const std::vector<GenericDecl*>& parameters);
	};

	struct GlobalDecl : public Decl {
		Ident* name;
		TypeSpec* tp;
		Expr* init;

		GlobalDecl(Ident* name, TypeSpec* tp, Expr* init, mist::Pos pos);
	};


	struct LocalDecl : public Decl {
		Pattern* name;
		std::vector<TypeSpec*> sp;
		std::vector<Expr*> init;
		bool is_self{false};

		LocalDecl(Pattern *name, const std::vector<TypeSpec *> &spec, const std::vector<Expr *> &init, mist::Pos pos);

		const std::vector<Expr*>& expr();
		const std::vector<TypeSpec*>& spec();
	};


	// x, y, z : i32, f32, string = foo()
	// x, y, z : i32, f32, string = 1, 2.0, "Hello, World"
//	struct MultiLocalDecl : public Decl {
//
//		std::vector<Pattern*> lvalues;
//
//		MultiLocalDecl(const std::vector<Pattern*>& lvalues, const std::vector<TypeSpec*>& sp,
//			const std::vector<Expr*>& init, mist::Pos pos);
//	};

	typedef LocalDecl FieldDecl;

	struct SelfFieldDecl : public FieldDecl {
		Mutability mut;

		SelfFieldDecl(Mutability mut, mist::Pos pos);
	};

	struct StructDecl : public Decl {
		std::vector<FieldDecl*> fields;
		std::vector<TypeSpec*> derives;
		WhereClause* where{nullptr};
		Generics* generics{nullptr};

		mist::Scope* scope;


		StructDecl(Ident* name, const std::vector<FieldDecl*>& fields, const std::vector<TypeSpec*>& derives, WhereClause* where, Generics* gen, mist::Pos pos);
	};

	struct ClassDecl : public StructDecl {
		std::vector<TypeSpec*> parents; // multiple inheritance

		ClassDecl(Ident* name, const std::vector<FieldDecl*>& fields, const std::vector<TypeSpec*>& derives, WhereClause* where, Generics* gen, const std::vector<TypeSpec*>& parents, mist::Pos pos);
	};
	
	struct FunctionDecl : public Decl {
		std::vector<FieldDecl*> parameters;
		std::vector<TypeSpec*> returns;
		Expr* body{nullptr};
		Generics* generics{nullptr};
		mist::Scope* paramScope;
		mist::Scope* bodyScope;

		FunctionDecl(Ident* ident, const std::vector<FieldDecl*>& params,
					 const std::vector<TypeSpec*>& rets, Expr* body, Generics* gen, mist::Pos pos);
	};

	struct OpFunctionDecl : public Decl {
		Op op;	
		std::vector<FieldDecl*> parameters;
		std::vector<TypeSpec*> returns;
		Expr* body{nullptr};
		Generics* generics{nullptr};
		mist::Scope* paramScope;
		mist::Scope* bodyScope;

		OpFunctionDecl(Op name, const std::vector<FieldDecl*>& params,
					 const std::vector<TypeSpec*>& rets, Expr* body, Generics* gen, mist::Pos pos);
	};

	struct TypeClassDecl : public Decl {
		std::vector<Decl*> members;		
		Generics* generics;
		mist::Scope* scope;

		TypeClassDecl(Ident* ident, const std::vector<Decl*>& members, Generics* gen, mist::Pos pos);
	};
	
	struct UseDecl : public Decl {
		Path* path;	
		std::vector<Path*> fields;

		ast::Module* module; // the module this use imports
							/// this can be either a file module
							/// or a multi file module

		UseDecl(Ident* ident, Path* path, const std::vector<Path*> fields, mist::Pos pos);
	};
	
	struct ImplDecl : public Decl {
		std::vector<FunctionDecl*> methods;		
		Generics* generics;
		mist::Scope* scope;

		ImplDecl(Ident* ident, const std::vector<FunctionDecl*>& methods, Generics* gen, mist::Pos pos);
	};

	enum VariantMemberKind {
		VariantIdent,
		VariantStruct
	};

	struct VariantMemberDecl : public Decl {
		VariantMemberKind ekind;
		Expr* init{nullptr};
		std::vector<TypeSpec*> types;
		mist::Scope* scope;

		VariantMemberDecl(Ident* name, VariantMemberKind ekind, mist::Pos pos, const
			std::vector<TypeSpec*>& types = std::vector<TypeSpec*>(), Expr* init = nullptr);
	};

	struct VariantDecl : public Decl {
		std::vector<VariantMemberDecl*> members;
		Generics* generics;
		mist::Scope* scope;

		VariantDecl(Ident *name, const std::vector<VariantMemberDecl *> &members, Generics *gen, mist::Pos pos);
	};

}
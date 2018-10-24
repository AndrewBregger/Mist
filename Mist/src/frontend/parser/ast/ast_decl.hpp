#pragma once

#include "ast_common.hpp"

namespace ast {

	struct Type;
	struct Expr;
	struct TypeSpec;

	enum DeclKind {
		Local,
		MultiLocal,
		Struct,
		TypeClass,
		Function,
		Use, // module system
		Impl,
		Generic
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
	};

	struct GenericDecl :  public Decl {
		TypeSpec* bounds;

		GenericDecl(Ident* name, TypeSpec* bounds, mist::Pos pos);
	};

	struct Generics {
		std::vector<GenericDecl*> parameters;

		Generics(const std::vector<GenericDecl*>& parameters);
	};


	struct LocalDecl : public Decl {
		TypeSpec* sp{nullptr};
		Expr* init{nullptr};

		LocalDecl(Ident* name, TypeSpec* spec, Expr* init, mist::Pos pos);

		Expr* expr();
		TypeSpec* spec();
	};

	// x, y, z : i32, f32, string = foo()
	// x, y, z : i32, f32, string = 1, 2.0, "Hello, World"
	struct MultiLocalDecl : public Decl {

		std::vector<Ident*> names;
		std::vector<TypeSpec*> sps;
		std::vector<Expr*> inits;

		MultiLocalDecl(const std::vector<Ident*>& names, const std::vector<TypeSpec*>& sp, 
			const std::vector<Expr*>& init, mist::Pos pos);
	};

	typedef LocalDecl FieldDecl;
	
	struct StructDecl : public Decl {
		std::vector<FieldDecl*> fields;
		std::vector<TypeSpec*> derives;
		WhereClause* where{nullptr};
		Generics* generics{nullptr};

		StructDecl(Ident* name, const std::vector<FieldDecl*>& fields, const std::vector<TypeSpec*>& derives, WhereClause* where, Generics* gen, mist::Pos pos);
	};
	
	struct FunctionDecl : public Decl {
		std::vector<FieldDecl*> parameters;
		std::vector<TypeSpec*> returns;
		Expr* body{nullptr};
		Generics* generics{nullptr};

		FunctionDecl(Ident* ident, const std::vector<FieldDecl*>& params,
					 const std::vector<TypeSpec*>& rets, Generics* gen, mist::Pos pos);
	};

	struct TypeClassDecl : public Decl {
		std::vector<Decl*> members;		
		Generics* generics;

		TypeClassDecl(Ident* ident, const std::vector<Decl*>& members, Generics* gen, mist::Pos pos);
	};
	
	struct UseDecl : public Decl {
		Path* path;	
		std::vector<Path*> fields;

		UseDecl(Ident* ident, Path* path, const std::vector<Path*> fields, mist::Pos pos);
	};
	
	struct ImplDecl : public Decl {
		std::vector<FunctionDecl*> methods;		
		Generics* generics;

		ImplDecl(Ident* ident, const std::vector<FunctionDecl*>& methods, Generics* gen, mist::Pos pos);
	};

}
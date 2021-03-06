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
		OpFunction,
		Use, // module system
		Impl,
		Generic,
		Enum,
		EnumMember
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


	struct LocalDecl : public Decl {
		TypeSpec* sp{nullptr};
		Expr* init{nullptr};
		bool is_self{false};

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
					 const std::vector<TypeSpec*>& rets, Expr* body, Generics* gen, mist::Pos pos);
	};

	struct OpFunctionDecl : public Decl {
		Op op;	
		std::vector<FieldDecl*> parameters;
		std::vector<TypeSpec*> returns;
		Expr* body{nullptr};
		Generics* generics{nullptr};

		OpFunctionDecl(Op name, const std::vector<FieldDecl*>& params,
					 const std::vector<TypeSpec*>& rets, Expr* body, Generics* gen, mist::Pos pos);
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

	enum EnumDeclKind {
		EnumIdent,
		EnumStruct
	};

	struct EnumMemberDecl : public Decl {
		EnumDeclKind ekind;
		Expr* init{nullptr};
		std::vector<TypeSpec*> types;

		EnumMemberDecl(Ident* name, EnumDeclKind ekind, mist::Pos pos, const 
			std::vector<TypeSpec*>& types = std::vector<TypeSpec*>(), Expr* init = nullptr);
	};

	struct EnumDecl : public Decl {
		std::vector<EnumMemberDecl*> members;
		Generics* generics;

		EnumDecl(Ident* name, const std::vector<EnumMemberDecl*> members, Generics* gen, mist::Pos pos);
	};

}
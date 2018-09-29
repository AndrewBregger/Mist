#pragma once

#include "ast_common.hpp"

namespace ast {

	struct Type;
	struct Expr;

	enum DeclKind {
		Local,
		Struct,
		TypeClass,
		Function,
		Use, // module system
		Impl,
	};

	struct Decl {
		DeclKind k;
		Type* t{nullptr};

		Decl(DeclKind k);

		DeclKind kind();

		Type* type();
	};

	struct LocalDecl : public Decl {

	};
	
	struct StructDecl : public Decl {

	};
	
	struct TypeClassDecl : public Decl {

	};
	
	struct FunctionDecl : public Decl {

	};
	
	struct UseDecl : public Decl {

	};
	
	struct ImplDecl : public Decl {

	};

}
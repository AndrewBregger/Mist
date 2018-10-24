#include "ast_decl.hpp"

#include "ast_common.hpp"

namespace ast {
	Decl::Decl(Ident* name, DeclKind k, mist::Pos pos) : name(name), k(k), pos(pos) {

	}

	DeclKind Decl::kind() {
		return k;
	}

	Type* Decl::type() {
		return t;
	}

	GenericDecl::GenericDecl(Ident* name, TypeSpec* bounds, mist::Pos pos) :
		Decl(name, Generic, pos), bounds(bounds) {}

	Generics::Generics(const std::vector<GenericDecl*>& parameters) : parameters(parameters) {

	}

	LocalDecl::LocalDecl(Ident* name, TypeSpec* spec, Expr* init, mist::Pos pos) : 
		Decl(name, Local, pos), sp(spec), init(init) {

	}

	Expr* LocalDecl::expr() {
		return init;
	}

	TypeSpec* LocalDecl::spec() {
		return sp;
	}

	MultiLocalDecl::MultiLocalDecl(const std::vector<Ident*>& names, const std::vector<TypeSpec*>& sps, 
		const std::vector<Expr*>& inits, mist::Pos pos) : Decl(nullptr, MultiLocal, pos), names(names), sps(sps), inits(inits){

	}

	StructDecl::StructDecl(Ident* name, const std::vector<FieldDecl*>& fields, const std::vector<TypeSpec*>& derives, WhereClause* where, Generics* gen, mist::Pos pos) :
		Decl(name, Struct, pos), fields(fields), derives(derives), where(where), generics(gen) {
	}

	FunctionDecl::FunctionDecl(Ident* ident, const std::vector<FieldDecl*>& params,
					const std::vector<TypeSpec*>& rets, Generics* gen, mist::Pos pos) :
			Decl(ident, Function, pos), parameters(params), returns(rets), generics(gen) {
	}

	TypeClassDecl::TypeClassDecl(Ident* ident, const std::vector<Decl*>& members, Generics* gen, mist::Pos pos) :
		Decl(ident, TypeClass, pos), members(members), generics(gen) {
	}

	UseDecl::UseDecl(Ident* ident, Path* path, const std::vector<Path*> fields, mist::Pos pos) :
		Decl(ident, Use, pos), path(path), fields(fields) {
	}

	ImplDecl::ImplDecl(Ident* ident, const std::vector<FunctionDecl*>& methods, Generics* gen, mist::Pos pos) :
		Decl(ident, Impl, pos), methods(methods), generics(gen) {
	}
}
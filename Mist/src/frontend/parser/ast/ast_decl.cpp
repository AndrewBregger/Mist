#include "ast_decl.hpp"

#include "ast_common.hpp"

#define ToString(x) #x

namespace ast {
	const static std::vector<std::string> decl_strings = {
		ToString(Local),
		ToString(MultiLocal),
		ToString(Struct),
		ToString(TypeClass),
		ToString(Function),
		ToString(OpFunction),
		ToString(Use),
		ToString(Impl),
		ToString(Generic),
		ToString(Enum),
		ToString(EnumMember)
	};

	Decl::Decl(Ident* name, DeclKind k, mist::Pos pos) : name(name), k(k), pos(pos) {

	}

	DeclKind Decl::kind() {
		return k;
	}

	Type* Decl::type() {
		return t;
	}

	const std::string& Decl::string() {
		return decl_strings[k];
	}

	GenericDecl::GenericDecl(Ident* name, const std::vector<TypeSpec*>& bounds, mist::Pos pos) :
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
		const std::vector<Expr*>& inits, mist::Pos pos) : Decl(nullptr, MultiLocal, pos), names(names), sps(sps), inits(inits) {

	}

	StructDecl::StructDecl(Ident* name, const std::vector<FieldDecl*>& fields, const std::vector<TypeSpec*>& derives, WhereClause* where, Generics* gen, mist::Pos pos) :
		Decl(name, Struct, pos), fields(fields), derives(derives), where(where), generics(gen) {
	}

	FunctionDecl::FunctionDecl(Ident* ident, const std::vector<FieldDecl*>& params,
		const std::vector<TypeSpec*>& rets, Expr* body, Generics* gen, mist::Pos pos) :
		Decl(ident, Function, pos), parameters(params), returns(rets), body(body), generics(gen) {
	}

	OpFunctionDecl::OpFunctionDecl(Op name, const std::vector<FieldDecl*>& params,
		const std::vector<TypeSpec*>& rets, Expr* body, Generics* gen, mist::Pos pos) : Decl(nullptr, OpFunction, pos), op(name), parameters(params), returns(rets), generics(gen), body(body) {
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

	EnumMemberDecl::EnumMemberDecl(Ident* name, EnumDeclKind ekind, mist::Pos pos,
		const std::vector<TypeSpec*>& types, Expr* init) : Decl(name, EnumMember, pos),
		types(types), init(init), ekind(ekind) {

	}

	EnumDecl::EnumDecl(Ident* name, const std::vector<EnumMemberDecl*> members, Generics* gen,
		mist::Pos pos) : Decl(name, Enum, pos), members(members), generics(gen) {
	}
}
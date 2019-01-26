#include "ast_decl.hpp"

#include "ast_common.hpp"

#define ToString(x) #x

namespace ast {
	const static std::vector<std::string> decl_strings = {
        ToString(Global),
		ToString(Local),
		ToString(SelfField),
		ToString(Struct),
		ToString(Class),
		ToString(TypeClass),
		ToString(Function),
		ToString(OpFunction),
		ToString(Use),
		ToString(Impl),
		ToString(Generic),
		ToString(Variant),
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

	mist::Scope *Decl::get_scope() {
	    switch(kind()) {
	    	case Struct:
            case Class: {
	    		auto decl = static_cast<StructDecl*>(this);
	    		return decl->scope;
	    	}
	    	case TypeClass: {
				auto decl = static_cast<TypeClassDecl*>(this);
				return decl->scope;
	    	}
	    	case Function: {
				auto decl = static_cast<FunctionDecl*>(this);
				return decl->paramScope;
	    	}
	    	case OpFunction: {
				auto decl = static_cast<OpFunctionDecl*>(this);
				return decl->paramScope;
	    	}
	    	case Use: {
				auto decl = static_cast<UseDecl*>(this);
				return decl->module->scope;
	    	}
	    	case Impl: {
				auto decl = static_cast<ImplDecl*>(this);
				return decl->scope;
	    	}
	    	case Generic: {
				auto decl = static_cast<GenericDecl*>(this);
				return nullptr;
	    	}
	    	case Variant: {
				auto decl = static_cast<VariantDecl*>(this);
				return decl->scope;
	    	}
	    	case VariantMember: {
				auto decl = static_cast<VariantMemberDecl*>(this);
				return decl->scope;
	    	}
	    	default:
	    		break;
		}
		return nullptr;
	}

	GenericDecl::GenericDecl(Ident* name, const std::vector<TypeSpec*>& bounds, mist::Pos pos) :
		Decl(name, Generic, pos), bounds(bounds) {}

	Generics::Generics(const std::vector<GenericDecl*>& parameters) : parameters(parameters) {

	}

	GlobalDecl::GlobalDecl(Ident *name, TypeSpec *tp, Expr *init, mist::Pos pos) : Decl(name, Global, pos), name(name), tp(tp), init(init) {

	}

	LocalDecl::LocalDecl(Pattern *name, const std::vector<TypeSpec *> &spec, const std::vector<Expr *> &init,
						 mist::Pos pos) :
		Decl(nullptr, Local, pos), name(name), sp(spec), init(init) {
	}

	const std::vector<Expr*>& LocalDecl::expr() {
		return init;
	}

	const std::vector<TypeSpec*>& LocalDecl::spec() {
		return sp;
	}

	ClassDecl::ClassDecl(Ident *name, const std::vector<FieldDecl *> &fields, const std::vector<TypeSpec *> &derives,
						 WhereClause *where, Generics *gen, const std::vector<TypeSpec *> &parents, mist::Pos pos) :
						 StructDecl(name, fields, derives, where, gen, pos), parents(parents) {
		k = Class;
	}

	SelfFieldDecl::SelfFieldDecl(Mutability mut, mist::Pos pos) :
		LocalDecl(nullptr, std::vector<TypeSpec*>(), std::vector<ast::Expr*>(), pos), mut(mut) {
		k = SelfField;
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

	VariantMemberDecl::VariantMemberDecl(ast::Ident *name, ast::VariantMemberKind ekind, mist::Pos pos,
										 const std::vector<ast::TypeSpec *> &types, ast::Expr *init) : Decl(name, VariantMember, pos),
										 ekind(ekind), types(types), init(init) {
	}

	VariantDecl::VariantDecl(Ident *name, const std::vector<VariantMemberDecl *> &members, Generics *gen,
							 mist::Pos pos) : Decl(name, Variant, pos), members(members), generics(gen) {
	}
}
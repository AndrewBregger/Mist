#include "ast_typespec.hpp"

#define ToString(x) #x

namespace ast {

	const static std::vector<std::string> spec_names = {
		ToString(Named),
		ToString(TupleType),
		ToString(FunctionType),
		ToString(TypeClassType),
		ToString(Array),
		ToString(DynamicArray),
		ToString(Map),
		ToString(Pointer),
		ToString(Reference),
		ToString(MutableType),
		ToString(Path),
		ToString(Unit)
	};


	TypeSpec::TypeSpec(TypeSpecKind k, Mutability mut, mist::Pos p) : TypeSpec(nullptr, k, mut, p) {}

	TypeSpec::TypeSpec(TypeSpec *base, TypeSpecKind k, Mutability mut, mist::Pos p)
			: k(k), p(p), mut(mut), base(base) { }

	const std::string& TypeSpec::name() {
		return spec_names[k];
	}

	GenericParameters::GenericParameters(const std::vector<Expr*>& expr) : exprs(expr) {
	}

	NamedSpec::NamedSpec(Ident *name, GenericParameters *params, Mutability mut, mist::Pos pos) :
			TypeSpec(Named, mut, pos), name(name), params(params) {

	}

	TupleSpec::TupleSpec(const std::vector<TypeSpec *> &types, Mutability mut, mist::Pos pos) :
			TypeSpec(TupleType, mut, pos), types(types) {
	}

	FunctionSpec::FunctionSpec(const std::vector<TypeSpec*>& parameters, const std::vector<TypeSpec*>& returns,
		mist::Pos pos) : TypeSpec(FunctionType, Immutable, pos), parameters(parameters), returns(returns) {
	}

	TypeClassSpec::TypeClassSpec(NamedSpec* name, mist::Pos pos) : TypeSpec(TypeClassType, Immutable, pos), name(name) {
	}

	ArraySpec::ArraySpec(TypeSpec *element, Expr *size, Mutability mut, mist::Pos pos) : TypeSpec(element, Array, mut,
																								 pos), size(size) {
	}

//	DynamicArraySpec::DynamicArraySpec(TypeSpec* element, mist::Pos pos) : TypeSpec(element, DynamicArray, MutableType, pos) {
//	}

	MapSpec::MapSpec(TypeSpec *key, TypeSpec *value, Mutability mut, mist::Pos pos) : TypeSpec(Map, mut, pos), key(key), value(value) {
	}

	PointerSpec::PointerSpec(TypeSpec *base, Mutability mut, mist::Pos pos) : TypeSpec(base, Pointer, mut, pos) {
	}

	ReferenceSpec::ReferenceSpec(TypeSpec *base, Mutability mut, mist::Pos pos) : TypeSpec(base, Reference, mut, pos) {
	}

//	MutableSpec::MutableSpec(TypeSpec* base, mist::Pos pos) : TypeSpec(base, MutableType, Immutable, pos) {
//	}

	PathSpec::PathSpec(const std::vector<NamedSpec *> &path, Mutability mut, mist::Pos pos) : TypeSpec(Path, mut, pos), path(path) {
	    for(auto x : this->path)
	    	x->mut = mut;
	}

	UnitSpec::UnitSpec(mist::Pos pos) : TypeSpec(Unit, Mutable, pos) {}
}

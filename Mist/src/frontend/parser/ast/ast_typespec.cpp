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
		ToString(Constant),
		ToString(Path),
		ToString(Unit)
	};

	
	TypeSpec::TypeSpec(TypeSpecKind k, mist::Pos p) : k(k), p(p) { } TypeSpec::TypeSpec(TypeSpec* base, TypeSpecKind k, mist::Pos p) : k(k), p(p), base(base) { }

	const std::string& TypeSpec::name() {
		return spec_names[k];
	}

	GenericParameters::GenericParameters(TypeSpec* type) {

	}

	GenericParameters::GenericParameters(Expr* expr) {

	}

	NamedSpec::NamedSpec(Ident* name, GenericParameters* params, mist::Pos pos) :
		TypeSpec(Named, pos), name(name), params(params) {

	}

	TupleSpec::TupleSpec(const std::vector<TypeSpec*>& types, mist::Pos pos) :
		TypeSpec(TupleType, pos), types(types) {
	}

	FunctionSpec::FunctionSpec(const std::vector<TypeSpec*>& parameters, const std::vector<TypeSpec*>& returns,
		mist::Pos pos) : TypeSpec(FunctionType, pos), parameters(parameters), returns(returns) {
	}

	TypeClassSpec::TypeClassSpec(NamedSpec* name, mist::Pos pos) : TypeSpec(TypeClassType, pos), name(name) {
	}

	ArraySpec::ArraySpec(TypeSpec* element, IntegerConstExpr* size, mist::Pos pos) : TypeSpec(element, Array, pos), size(size) {
	}

	DynamicArraySpec::DynamicArraySpec(TypeSpec* element, mist::Pos pos) : TypeSpec(element, DynamicArray, pos) {
	}

	MapSpec::MapSpec(TypeSpec* key, TypeSpec* value, mist::Pos pos) : TypeSpec(Map, pos), key(key), value(value) {
	}

	PointerSpec::PointerSpec(TypeSpec* base, mist::Pos pos) : TypeSpec(base, Pointer, pos) {
	}

	ReferenceSpec::ReferenceSpec(TypeSpec* base, mist::Pos pos) : TypeSpec(base, Reference, pos) {
	}

	ConstantSpec::ConstantSpec(TypeSpec* base, mist::Pos pos) : TypeSpec(base, Constant, pos) {
	}

	PathSpec::PathSpec(const std::vector<NamedSpec*>& path, mist::Pos pos) : TypeSpec(Path, pos), path(path) {
	}

	UnitSpec::UnitSpec(mist::Pos pos) : TypeSpec(Unit, pos) {}
}
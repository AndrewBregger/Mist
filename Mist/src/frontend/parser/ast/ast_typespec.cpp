#include "ast_typespec.hpp"


namespace ast {
		TypeSpec::TypeSpec(TypeSpecKind k, mist::Pos p) : k(k), p(p) {

		}

		TypeSpec::TypeSpec(TypeSpec* base, TypeSpecKind k, mist::Pos p) : k(k), p(p), base(base) {

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
}
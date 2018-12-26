#pragma once

#include "ast_common.hpp"

namespace ast {

	struct Expr;
	struct IntegerConstExpr;

	enum TypeSpecKind  {
		Named,
		TupleType,
		FunctionType,
		TypeClassType,
		Array,
		DynamicArray,
		Map,
		Pointer,
		Reference,
		Constant,
		Path,
		Unit
	};

	struct TypeSpec {
		TypeSpecKind k;
		mist::Pos p;
		Mutablity mut;
		TypeSpec* base{nullptr};

		TypeSpec(TypeSpecKind k, Mutablity mut, mist::Pos p);
		TypeSpec(TypeSpec *base, TypeSpecKind k, Mutablity mut, mist::Pos p);

		const std::string& name();

		inline mist::Pos pos() { return p; }
	};


	enum GenericParameterType {
		TypeParam,
		ExprParam
	};

	struct GenericParameters {
		std::vector<Expr*> exprs;

		GenericParameters(const std::vector<Expr*>& expr);
	};

	struct NamedSpec : public TypeSpec {
		Ident* name;
		GenericParameters* params;

		NamedSpec(Ident *name, GenericParameters *params, Mutablity mut, mist::Pos pos);
	};

	struct TupleSpec : public TypeSpec  {
		std::vector<TypeSpec*> types;

		TupleSpec(const std::vector<TypeSpec *> &types, Mutablity mut, mist::Pos pos);
	};

	struct FunctionSpec : public TypeSpec  {
		std::vector<TypeSpec*> parameters;
		std::vector<TypeSpec*> returns;

		FunctionSpec(const std::vector<TypeSpec*>& parameters, const std::vector<TypeSpec*>& returns,
			mist::Pos pos);
	};

	struct TypeClassSpec : public TypeSpec  {
		NamedSpec* name;

		TypeClassSpec(NamedSpec* name, mist::Pos pos);
	};

	struct ArraySpec : public TypeSpec {
		TypeSpec* element;

		Expr* size;

		ArraySpec(TypeSpec *element, Expr *size, Mutablity mut, mist::Pos pos);
	};
//
//	struct DynamicArraySpec : public TypeSpec  {
//		TypeSpec* element;
//		DynamicArraySpec(TypeSpec* element, mist::Pos pos);
//	};

	struct MapSpec : public TypeSpec  {
		TypeSpec* key,* value;
		MapSpec(TypeSpec *key, TypeSpec *value, Mutablity mut, mist::Pos pos);
	};

	struct PointerSpec : public TypeSpec  {
		PointerSpec(TypeSpec *base, Mutablity mut, mist::Pos pos);
	};

	struct ReferenceSpec : public TypeSpec  {
		ReferenceSpec(TypeSpec *base, Mutablity mut, mist::Pos pos);
	};

	struct ConstantSpec : public TypeSpec  {
		ConstantSpec(TypeSpec* base, mist::Pos pos);
	};

	struct PathSpec : public TypeSpec  {
		std::vector<NamedSpec*> path;
		PathSpec(const std::vector<NamedSpec *> &path, Mutablity mut, mist::Pos pos);
	};

	struct UnitSpec : public TypeSpec {
		UnitSpec(mist::Pos pos);
	};
}

#pragma once

#include "ast_common.hpp"

namespace ast {
	struct Type;
	struct Decl;
	struct TypeSpec;

	enum ExprKind {
		// Represents an identifier.
		Value,
		Tuple,
		
		// Constants
		IntegerConst,
		FloatConst,
		StringConst,
		BooleanConst,
		CharConst,

		// Arithmetic operations
		Binary,
		Unary,


		// Control Flow
		If,
		While,
		Loop,
		For,
		Match,

		// other
		Decl,
		Parenthesis,
		Selector,

		Break,
		Continue,
		Return,
		Cast,
		Range,
		Slice,
		TupleIndex,

		Assignment,

		Block
	};

	enum BinaryOp {
		Plus,
		Minus,
		Slash, 
		Percent,
		Astrick,
		AstrickAstrick,
		LessLess,
		GreaterGreater,
		Pipe,
		Carrot,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,
		EqualEqual,
		BangEqual,
	};

	enum UnaryOp {
		Minus,
		Bang,
		Tilde,
		Ampersand,
		Astrick
	};

	enum ConstantType {
		I8,
		I16,
		I32,
		I64,
		U8,
		U16,
		U32,
		U64,
		F32,
		F64,
		Char
	};

	struct Expr {
		ExprKind k;
		Type* t;
		mist::Pos p;

		Expr(ExprKind k, mist::Pos p);

		ExprKind kind();
		Type* type();
		mist::Pos pos();
	};

	struct ValueExpr : public Expr {
		Ident* name;

		Value(Ident* name);
	};

	struct TupleExpr : public Expr {
		std::vector<Expr*> values;

		Tuple(const std::vector<Expr*>& values, mist::Pos pos);
	};

	struct IntegerConstExpr : public Expr {
		i64 value{0};
		ConstantType cty;

		IntegerConst(i64 val, ConstantType cty, mist::Pos pos);
	};

	struct FloatConstExpr : public Expr {
		f64 value{0};
		ConstantType cty;

		FloatConst(f64 val, ConstantType cty, mist::Pos pos);
	};

	struct StringConstExpr : public Expr {
		std::string value;

		StringConst(const std::string& val, mist::Pos pos);
	};

	struct BooleanConstExpr : public Expr {
		bool value;

		BooleanConst(bool val, mist::Pos pos);
	};

	struct CharConstExpr : public Expr {
		char value;

		CharConst(char val, mist::Pos pos);
	};

	struct BinaryExpr : public Expr {
		BinaryOp op;
		Expr* lhs;
		Expr* rhs;

		BinaryExpr(BinaryOp op, Expr* lhs, Expr* rhs, mist::Pos pos);
	};

	struct UnaryExpr : public Expr {
		UnaryOp op;
		Expr* expr;
		
		UnaryExpr(UnaryOp op, Expr* expr, mist::Pos pos);
	};

	struct IfExpr : public Expr {
		Expr* cond;
		Expr* body;

		IfExpr(Expr* cond, Expr* body, mist::Pos pos);
	};

	struct WhileExpr : public Expr {
		Expr* cond;
		Expr* body;

		WhileExpr(Expr* cond, Expr* body, mist::Pos pos);
	};

	struct LoopExpr : public Expr {
		Expr* body;

		LoopExpr(Expr* body, mist::Pos pos);
	};

	struct ForExpr : public Expr {
		Expr* index;
		Expr* expr;
		Expr* body;

		ForExpr(Expr* index, Expr* expr, Expr* body, mist::Pos pos);
	};

	struct MatchArm {
		Expr* name;
		Ident* value;
		Expr* body;

		MatchArm(Expr* name, Ident* value, Expr* body);
	};

	struct MatchExpr : public Expr {
		Expr* cond;
		std::vector<MatchArm*> arms;

		MatchExpr(Expr* cond, const std::vector<MatchArm*>& arms, mist::Pos pos);
	};

	struct DeclExpr : public Expr {
		Decl* decl;

		DeclExpr(Decl* decl);
	};

	struct ParenthesisExpr : public Expr {
		Expr* operand;

		ParenthesisExpr(Expr* operand);
	};

	struct SelectorExpr : public Expr {
		Expr* operand;
		TypeSpec* element;

		SelectorExpr(Expr* operand, TypeSpec* element, mist::Pos pos);
	};

	struct BreakExpr : public Expr {
		BreakExpr(mist::Pos pos);
	};

	struct ContinueExpr : public Expr {
		ContinueExpr(mist::Pos pos);
	};

	struct ReturnExpr : public Expr {
		std::vector<Expr*> returns;

		ReturnExpr(const std::vector<Expr*> returns, mist::Pos pos);
	};

	struct CastExpr : public Expr {
		Expr* expr;
		TypeSpec* ty;

		CastExpr(Expr* expr, TypeSpec* ty, mist::Pos pos);
	};

	struct RangeExpr : public Expr {
		Expr* low;
		Expr* high;
		Expr* count;

		RangeExpr(Expr* low, Expr* high, Expr* count, mist::Pos pos);
	};

	struct SliceExpr : public Expr {
		Expr* low;
		Expr* high;

		SliceExpr(Expr* low, Expr* high, mist::Pos pos);
	};

	struct TupleIndexExpr : public Expr {
		Expr* operand;
		i32 index;

		TupleIndexExpr(Expr* operand, i32 index, mist::Pos pos);
	};

	struct AssignmentExpr : public Expr {
		std::vector<Expr*> lvalues;
		Expr* expr;

		AssignmentExpr(const std::vector<Expr*> lvalues, Expr* expr, mist::Pos pos);
	};

	struct BlockExpr : public Expr {
		std::vector<Expr*> elements;

		BlockExpr(const std::vector<Expr*> elements, mist::Pos pos);
	};
}
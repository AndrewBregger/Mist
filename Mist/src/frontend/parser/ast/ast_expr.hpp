#pragma once

#include "ast_common.hpp"
#include "../tokenizer/token.hpp"

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
		DeclDecl,
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

		Block,
		StructLiteral
	};

	enum BinaryOp {
		Plus,
		BMinus,
		Slash, 
		Percent,
		BAstrick,
		AstrickAstrick,
		BAmpersand,
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
		UMinus,
		Bang,
		Tilde,
		UAmpersand,
		UAstrick
	};

	UnaryOp from_token(mist::TokenKind k);
	mist::TokenKind from_unary(ast::UnaryOp op);

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
	
	enum AssignmentOp {
		PlusEqual,
		MinusEqual,
		AstrickEqual,
		SlashEqual,
		PercentEqual,
		AstrickAstrickEqual,
		LessLessEqual,
		GreaterGreaterEqual,
		CarrotEqual,
		AmpersandEqual,
		PipeEqual,
	};

	struct Expr {
		ExprKind k;
		Type* t;
		mist::Pos p;

		Expr(ExprKind k, mist::Pos p);

		ExprKind kind();
		Type* type();
		mist::Pos pos();

		const std::string& name();
	};

	struct ValueExpr : public Expr {
		Ident* name;
		std::vector<Expr*> genericValues;

		ValueExpr(Ident* name, const std::vector<Expr*>& generics, mist::Pos pos);
	};

	struct TupleExpr : public Expr {
		std::vector<Expr*> values;

		TupleExpr(const std::vector<Expr*>& values, mist::Pos pos);
	};

	struct IntegerConstExpr : public Expr {
		i64 value{0};
		ConstantType cty;

		IntegerConstExpr(i64 val, ConstantType cty, mist::Pos pos);
	};

	struct FloatConstExpr : public Expr {
		f64 value{0};
		ConstantType cty;

		FloatConstExpr(f64 val, ConstantType cty, mist::Pos pos);
	};

	struct StringConstExpr : public Expr {
		std::string value;

		StringConstExpr(const std::string& val, mist::Pos pos);
	};

	struct BooleanConstExpr : public Expr {
		bool value;

		BooleanConstExpr(bool val, mist::Pos pos);
	};

	struct CharConstExpr : public Expr {
		char value;

		CharConstExpr(char val, mist::Pos pos);
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
		ast::Decl* decl;

		DeclExpr(ast::Decl* decl);
	};

	struct ParenthesisExpr : public Expr {
		Expr* operand;
		std::vector<Expr*> params;
		ParenthesisExpr(Expr* operand, const std::vector<Expr*>& params, mist::Pos pos);
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
		AssignmentOp op;
		std::vector<Expr*> lvalues;
		Expr* expr;

		AssignmentExpr(AssignmentOp op, const std::vector<Expr*> lvalues, Expr* expr, mist::Pos pos);
	};

	struct BlockExpr : public Expr {
		std::vector<Expr*> elements;

		BlockExpr(const std::vector<Expr*> elements, mist::Pos pos);
	};

	// struct StructLiteralExpr : public Expr {
	// 	std::vector<>
	// }
}
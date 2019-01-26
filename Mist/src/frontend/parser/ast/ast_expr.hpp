#pragma once

#include "ast_common.hpp"
#include "../tokenizer/token.hpp"
#include "ast_pattern.hpp"

namespace mist {
	class Scope;
	class Type;
}

namespace ast {
//	struct Type;
	struct Decl;
	struct LocalDecl;
	struct TypeSpec;
	struct Pattern;


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
		StructLiteral,
		Binding,
		UnitLit,

		SelfLit,

		Lambda,
		CompoundLiteral
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

	const std::string & get_binary_op_string(ast::BinaryOp op);

	enum UnaryOp {
		UMinus,
		UBang,
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
		Char,
		None
	};

	enum AssignmentOp {
		Equal,
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
		mist::Type* t;
		mist::Pos p;

		Expr(ExprKind k, mist::Pos p);

		ExprKind kind();
		mist::Type* type();
		mist::Pos pos();

		inline virtual bool is_literal() { return false; }

		mist::Scope* get_scope();

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

		inline virtual bool is_literal() override { return true; }
	};

	struct FloatConstExpr : public Expr {
		f64 value{0};
		ConstantType cty;

		FloatConstExpr(f64 val, ConstantType cty, mist::Pos pos);
		inline virtual bool is_literal() override { return true; }
	};

	struct StringConstExpr : public Expr {
		std::string value;

		StringConstExpr(const std::string& val, mist::Pos pos);
		inline virtual bool is_literal() override { return true; }
	};

	struct BooleanConstExpr : public Expr {
		bool value;

		BooleanConstExpr(bool val, mist::Pos pos);
		inline virtual bool is_literal() override { return true; }
	};

	struct CharConstExpr : public Expr {
		char value;

		CharConstExpr(char val, mist::Pos pos);
		inline virtual bool is_literal() override { return true; }
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

	/// For, while, if, loop, and match the body
	/// scope is only valid if the body expression
	/// is a block.
	///
	struct IfExpr : public Expr {
		Expr* cond;
		Expr* body;
		Expr* elif;
		mist::Scope* ifscope;
		mist::Scope* elscope;

		IfExpr(Expr *cond, Expr *body, ast::Expr *elif, mist::Pos pos);
	};

	struct WhileExpr : public Expr {
		Expr* cond;
		Expr* body;
		mist::Scope* scope;

		WhileExpr(Expr* cond, Expr* body, mist::Pos pos);
	};

	struct LoopExpr : public Expr {
		Expr* body;
		mist::Scope* scope;

		LoopExpr(Expr* body, mist::Pos pos);
	};

	struct ForExpr : public Expr {
		Pattern* index;
		Expr* expr;
		Expr* body;
		mist::Scope* scopeIndex;
		mist::Scope* scopeBody;

		ForExpr(Pattern *pat, Expr *expr, Expr *body, mist::Pos pos);
	};

	struct MatchArm {
		Expr* name;
		Ident* value;
		Expr* body;
		mist::Scope* scope;

		MatchArm(Expr* name, Ident* value, Expr* body);
	};

	struct MatchExpr : public Expr {
		Expr* cond;
		std::vector<MatchArm*> arms;
		mist::Scope* scope; /// does this need a scope?

		MatchExpr(Expr* cond, const std::vector<MatchArm*>& arms, mist::Pos pos);
	};

	struct DeclExpr : public Expr {
		ast::Decl* decl;

		DeclExpr(ast::Decl* decl);
	};

	struct ParenthesisExpr : public Expr {
		Expr* operand;
		std::vector<Expr*> params;
		ParenthesisExpr(Expr *operand, const std::vector<Expr *> &params, mist::Pos pos);
	};

	struct SelectorExpr : public Expr {
		Expr* operand;
		ValueExpr* element;

		SelectorExpr(Expr* operand, ValueExpr* element, mist::Pos pos);
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

		AssignmentExpr(AssignmentOp op, const std::vector<Expr*>& lvalues, Expr* expr, mist::Pos pos);
	};

	struct BlockExpr : public Expr {
		std::vector<Expr*> elements;
		mist::Scope* scope;

		BlockExpr(const std::vector<Expr*>& elements, mist::Pos pos);
	};

	struct BindingExpr : public Expr {
		ast::Expr* name;
		Expr* expr;

		BindingExpr(ast::Expr* name, Expr* expr, mist::Pos pos);
	};

	struct UnitExpr : public Expr {
		UnitExpr(mist::Pos pos);
	};

	struct SelfExpr : public Expr {
		SelfExpr(mist::Pos pos);
	};

	 struct StructLiteralExpr : public Expr {
	 	Expr* name;
	 	std::vector<Expr*> members;

	 	StructLiteralExpr(Expr* name, const std::vector<Expr*>& members, mist::Pos pos);
		 inline virtual bool is_literal() override { return true; }
	 };

	 struct LambdaExpr : public Expr {
		std::vector<LocalDecl*> fields;
		std::vector<TypeSpec*> returns;
		Expr* body;
		mist::Scope* scope;


		LambdaExpr(const std::vector<LocalDecl*>& fields, const std::vector<TypeSpec*>& returns, Expr* body, mist::Pos pos);
	 };

	 struct CompoundLiteralExpr : public Expr {
		std::vector<Expr*> elements;

		CompoundLiteralExpr(const std::vector<Expr*>& elements, mist::Pos pos);

		 inline virtual bool is_literal() override { return true; }
	 };
}

#include "ast_expr.hpp"
#include "ast_decl.hpp"

#define ToString(x) #x

namespace ast {

	const static std::vector<std::string> expr_strings = {
		ToString(Value),
		ToString(Tuple),
		ToString(IntegerConst),
		ToString(FloatConst),
		ToString(StringConst),
		ToString(BooleanConst),
		ToString(CharConst),
		ToString(Binary),
		ToString(Unary),
		ToString(If),
		ToString(While),
		ToString(Loop),
		ToString(For),
		ToString(Match),
		ToString(DeclDecl),
		ToString(Parenthesis),
		ToString(Selector),
		ToString(Break),
		ToString(Continue),
		ToString(Return),
		ToString(Cast),
		ToString(Range),
		ToString(Slice),
		ToString(TupleIndex),
		ToString(Assignment),
		ToString(Block),
		ToString(StructLiteral),
		ToString(Binding),
		ToString(UnitLit),
		ToString(SelfLit)
	};

	UnaryOp from_token(mist::TokenKind k) {
		switch(k) {
			case mist::Tkn_Minus:
				return UMinus;
			case mist::Tkn_Tilde:
				return Tilde;
			case mist::Tkn_Astrick:
				return UAstrick;
			case mist::Tkn_Bang:
				return Bang;
			case mist::Tkn_Ampersand:
				return UAmpersand;
			default:
				return UMinus;
		}
	}

	mist::TokenKind from_unary(ast::UnaryOp op) {
		switch(op) {
			case UMinus:
				return mist::Tkn_Minus;
			case Bang:
				return mist::Tkn_Bang;
			case Tilde:
				return mist::Tkn_Tilde;
			case UAmpersand:
				return mist::Tkn_Ampersand;
			case UAstrick:
				return mist::Tkn_Astrick;
			default:
				return mist::Tkn_Error;
		 }
	}

	ExprKind Expr::kind() { return k; }
	
	Type* Expr::type() { return t; }
	
	mist::Pos Expr::pos() { return p; }

	const std::string& Expr::name() {
		return expr_strings[k];
	}

	Expr::Expr(ExprKind k, mist::Pos p) : k(k), p(p) {}

	ValueExpr::ValueExpr(Ident* name, const std::vector<Expr*>& generics, mist::Pos pos) : Expr(Value, pos), name(name), genericValues(generics) {}
	
	TupleExpr::TupleExpr(const std::vector<Expr*>& values, mist::Pos pos) : Expr(Tuple, pos), values(values) {

	}
	
	IntegerConstExpr::IntegerConstExpr(i64 val, ConstantType cty, mist::Pos pos) : Expr(IntegerConst, pos),
		value(val), cty(cty) {}
	
	FloatConstExpr::FloatConstExpr(f64 val, ConstantType cty, mist::Pos pos) : Expr(FloatConst, pos),
		value(val), cty(cty) {	}
	
	StringConstExpr::StringConstExpr(const std::string& val, mist::Pos pos) : Expr(StringConst, pos), value(val) {
	}
	
	BooleanConstExpr::BooleanConstExpr(bool val, mist::Pos pos) : Expr(BooleanConst, pos), value(val) {
	}
	
	CharConstExpr::CharConstExpr(char val, mist::Pos pos) : Expr(CharConst, pos), value(val) {
	}
	
	BinaryExpr::BinaryExpr(BinaryOp op, Expr* lhs, Expr* rhs, mist::Pos pos) : Expr(Binary, pos),
		op(op), lhs(lhs), rhs(rhs) {}

	UnaryExpr::UnaryExpr(UnaryOp op, Expr* expr, mist::Pos pos) : Expr(Unary, pos), op(op), expr(expr) {
	}
	
	IfExpr::IfExpr(Expr* cond, Expr* body, mist::Pos pos) : Expr(If, pos), cond(cond), body(body) {
	}
	
	WhileExpr::WhileExpr(Expr* cond, Expr* body, mist::Pos pos) : Expr(While, pos), cond(cond), body(body) {
	}
	
	LoopExpr::LoopExpr(Expr* body, mist::Pos pos) : Expr(Loop, pos), body(body) {
	}
	
	ForExpr::ForExpr(Expr* index, Expr* expr, Expr* body, mist::Pos pos) : Expr(For, pos), index(index), expr(expr), body(body) {
	}
	
	MatchArm::MatchArm(Expr* name, Ident* value, Expr* body) : name(name), value(value), body(body) {
	}
	
	MatchExpr::MatchExpr(Expr* cond, const std::vector<MatchArm*>& arms, mist::Pos pos) : Expr(Match, pos), cond(cond), arms(arms) {
	}
	
	DeclExpr::DeclExpr(Decl* decl) : Expr(ExprKind::DeclDecl, decl->pos), decl(decl) {
	}
	
	ParenthesisExpr::ParenthesisExpr(Expr* operand, const std::vector<Expr*>& params, mist::Pos pos) : Expr(Parenthesis, pos), operand(operand), params(params){
	}
	
	SelectorExpr::SelectorExpr(Expr* operand, ValueExpr* element, mist::Pos pos) : Expr(Selector, pos), operand(operand), element(element) {
	}
	
	BreakExpr::BreakExpr(mist::Pos pos) : Expr(Break, pos) {
	}
	
	ContinueExpr::ContinueExpr(mist::Pos pos) : Expr(Continue, pos) {
	}
	
	ReturnExpr::ReturnExpr(const std::vector<Expr*> returns, mist::Pos pos) : Expr(Return, pos), returns(returns) {
	}
	
	CastExpr::CastExpr(Expr* expr, TypeSpec* ty, mist::Pos pos) : Expr(Cast, pos), expr(expr), ty(ty) {
	}
	
	RangeExpr::RangeExpr(Expr* low, Expr* high, Expr* count, mist::Pos pos) : Expr(Range, pos), low(low), high(high), count(count) {
	}
	
	SliceExpr::SliceExpr(Expr* low, Expr* high, mist::Pos pos) : Expr(Slice, pos), low(low), high(high) {
	}
	
	TupleIndexExpr::TupleIndexExpr(Expr* operand, i32 index, mist::Pos pos) : Expr(TupleIndex, pos), operand(operand), index(index) {
	}
	
	AssignmentExpr::AssignmentExpr(AssignmentOp op, const std::vector<Expr*> lvalues, Expr* expr, mist::Pos pos) : Expr(Assignment, pos), op(op), lvalues(lvalues), expr(expr) {
	}
	
	BlockExpr::BlockExpr(const std::vector<Expr*> elements, mist::Pos pos) : Expr(Block, pos), elements(elements) {
	}
	
	BindingExpr::BindingExpr(ast::Ident* name, Expr* expr, mist::Pos pos) : Expr(Binding, pos), name(name), expr(expr) {
	}
	
	UnitExpr::UnitExpr(mist::Pos pos) : Expr(UnitLit, pos) {
	}

	SelfExpr::SelfExpr(mist::Pos pos) : Expr(SelfLit, pos) {
	}
}
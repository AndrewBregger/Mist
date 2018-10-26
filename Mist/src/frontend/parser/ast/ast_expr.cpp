#include "ast_expr.hpp"

namespace ast {
	ExprKind Expr::kind();
	Type* Expr::type();
	mist::Pos Expr::pos();
	Expr::Expr(ExprKind k, mist::Pos p) {

	}

	Value::Value(Ident* name) : Decl(, pos) {

	}
	
	Tuple::Tuple(const std::vector<Expr*>& values, mist::Pos pos) : Decl(, pos) {

	}
	
	IntegerConstExpr::IntegerConstExpr(i64 val, ConstantType cty, mist::Pos pos) : Decl(, pos) {

	}
	
	FloatConstExpr::FloatConstExpr(f64 val, ConstantType cty, mist::Pos pos) : Decl(, pos) {

	}
	
	StringConstExpr::StringConstExpr(const std::string& val, mist::Pos pos) : Decl(, pos) {

	}
	
	BooleanConstExpr::BooleanConstExpr(bool val, mist::Pos pos) : Decl(, pos) {

	}
	
	CharConst::CharConst(char val, mist::Pos pos) : Decl(, pos) {

	}
	
	BinaryExpr::BinaryExpr(BinaryOp op, Expr* lhs, Expr* rhs, mist::Pos pos) : Decl(, pos) {

	}
	
	UnaryExpr::UnaryExpr(UnaryOp op, Expr* expr, mist::Pos pos) : Decl(, pos) {

	}
	
	IfExpr::IfExpr(Expr* cond, Expr* body, mist::Pos pos) : Decl(, pos) {

	}
	
	WhileExpr::WhilExpr(Expr* cond, Expr* body, mist::Pos pos) : Decl(, pos) {

	}
	
	LoopExpr::LoopExpr(Expr* body, mist::Pos pos) : Decl(, pos) {

	}
	
	ForExpr::ForExpr(Expr* index, Expr* expr, Expr* body, mist::Pos pos) : Decl(, pos) {

	}
	
	MatchArm::MatchArm(Expr* name, Ident* value, Expr* body) : Decl(, pos) {

	}
	
	MatchExpr::MatchExpr(Expr* cond, const std::vector<MatchArm*>& arms, mist::Pos pos) : Decl(, pos) {

	}
	
	DeclExpr::DeclExpr(Decl* decl) : Decl(, pos) {

	}
	
	ParenthesisExpr::ParenthesisExpr(Expr* operand) : Decl(, pos) {

	}
	
	SelectorExpr::SelectorExpr(Expr* operand, TypeSpec* element, mist::Pos pos) : Decl(, pos) {

	}
	
	BreakExpr::BreakExpr(mist::Pos pos) : Decl(, pos) {

	}
	
	ContinueExpr::ContinueExpr(mist::Pos pos) : Decl(, pos) {

	}
	
	ReturnExpr::ReturnExpr(const std::vector<Expr*> returns, mist::Pos pos) : Decl(, pos) {

	}
	
	CastExpr::CastExpr(Expr* expr, TypeSpec* ty, mist::Pos pos) : Decl(, pos) {

	}
	
	RangeExpr::RangeExpr(Expr* low, Expr* high, Expr* count, mist::Pos pos) : Decl(, pos) {

	}
	
	SliceExpr::SliceExpr(Expr* low, Expr* high, mist::Pos pos) : Decl(, pos) {

	}
	
	TupleIndexExpr::TupleIndexExpr(Expr* operand, i32 index, mist::Pos pos) : Decl(, pos) {

	}
	
	AssignmentExpr::AssignmentExpr(const std::vector<Expr*> lvalues, Expr* expr, mist::Pos pos) : Decl(, pos) {

	}
	
	BlockExpr::BlockExpr(const std::vector<Expr*> elements, mist::Pos pos) : Decl(, pos) {

	}
	
}
/*
How to handle newlines in the middle of an expression?
*/

#include "parser.hpp"
#include <algorithm>

#include <cstdarg>
#include "interpreter.hpp"
#include "ast/ast_decl.hpp"
#include "ast/ast_expr.hpp"
#include "ast/ast_typespec.hpp"
#include "ast/ast_printer.hpp"

// REMOVE ME!!!
#include <iostream>

namespace mist {
	Parser::Parser(mist::Interpreter* interp) : interp(interp), file(nullptr),
		scanner(new Scanner(interp)), curr(Tkn_Error, Pos()) {
	}

	// for now these arent different. They probabily will
	// later.
	ast::Module* Parser::parse_root(io::File* file) {
		return parse_module(file);
	}

	ast::Module* Parser::parse_module(io::File* file) {
		std::cout << "Parsing module" << std::endl;
		this->file = file;
		reset();

		auto e = parse_expr();

		std::cout << e << std::endl;
		ast::print(std::cout, e);

		auto module = new ast::Module(file);
		//// while we are not at the end of the file. 
		//// Try to parse a new declaration
		//while(current().kind() != mist::Tkn_Eof) {
		//	auto d = parse_toplevel_decl();
		//	if(d)
		//		module->add_decl(d);
		//	else {
		//		interp->report_error(current().pos(), "failed to find top level declaration");
		//		sync();
		//	}
		//}

		return module;
	}


	void Parser::reset() {
		// this needs to through an error
		if (!file) return;

		scanner->init(file);

		// get the first token.
		scanner->advance();
		curr = scanner->token();
		scanner->advance();
	}

	ast::Expr* Parser::parse_expr() {
		return parse_accoc_expr(1);
	}

	ast::Expr* Parser::parse_accoc_expr(i32 prec) {
		auto expr = parse_primary_expr();
		std::cout << __FUNCTION__ << " " << current() << std::endl;
		if(check(Tkn_Comma)) {
			std::vector<ast::Expr*> lvalues = { expr };
			auto pos = expr->pos();
			while (check(mist::Tkn_Comma)) {
				pos = pos + current().pos();
				advance();
				lvalues.push_back(parse_expr_with_res(NoStructLiterals));
				pos = pos + lvalues.back()->pos();
			}
			auto token = current();

			pos = pos + token.pos();

			if(!token.is_assignment()) {
				interp->report_error(current().pos(), "only assignment operators are allowed, found: %s",
					current().get_string().c_str());
			}
			advance();
			auto rhs = parse_expr();
			pos = pos + rhs->pos();
			return new ast::AssignmentExpr((ast::AssignmentOp) (token.kind() - mist::Tkn_Equal), lvalues, rhs, pos);
		}

		while(current().prec() >= prec) {
			auto token = current();
			i32 curr_prec = current().prec();
			advance();
			std::cout << "Current Token: " << token << " - " << current() << std::endl;
			if (curr_prec < prec) {
				std::cout << "Breaking from binary parsing" << std::endl;
				break;
			}

			if(!token.is_operator()) {
				interp->report_error(current().pos(), "expecting binary operator, found: %s", token.get_string().c_str());
			}

			auto rhs = parse_accoc_expr(curr_prec + 1);
			ast::BinaryOp op = (ast::BinaryOp) (token.kind() - mist::Tkn_Plus);
			expr = new ast::BinaryExpr(op, expr, rhs, expr->pos() + token.pos() + rhs->pos());
		}

		return expr;
	}

	ast::Expr* Parser::parse_expr_with_res(Restriction res) {
		auto old = this->res;
		this->res = res;
		auto e = parse_accoc_expr(2);
		this->res = old;
		return e;
	}

	ast::Expr* Parser::parse_primary_expr() {
		auto c = current();
		switch(current().kind()) {
			case Tkn_Bang:
			case Tkn_Tilde:
			case Tkn_Minus:
			case Tkn_Astrick:
			case Tkn_Ampersand: {
				advance();
				auto e = parse_primary_expr();
				return new ast::UnaryExpr(ast::from_token(c.kind()), e, c.position + e->p);
			}
			default:
				return parse_atomic_expr();
		}
	}

	ast::Expr* Parser::parse_atomic_expr() {
		auto bottom = parse_bottom_expr();
		return parse_suffix_expr(bottom);
	}

	ast::Expr* Parser::parse_bottom_expr() {
		auto token = current();
		std::cout << __FUNCTION__ << " " << token << std::endl;
		switch(token.kind()) {
			case Tkn_OpenParen: {
				advance();
				auto expr = parse_expr();
				if(check(Tkn_Comma)) {
					std::vector<ast::Expr*> exprs;
					auto pos = token.pos();
					while(Tkn_Comma) {
						pos = pos + current().pos();
						advance();
						exprs.push_back(parse_expr());
						pos = pos + exprs.back()->pos();
					}
					// the close paren
					pos = pos + current().pos();
					expr = new ast::TupleExpr(exprs, pos);
				}
				expect(Tkn_CloseParen);
				return expr;
			}
			case Tkn_OpenBrace:
				// array or map literal.
				interp->report_error(current().pos(), "array literal not implemented");
				break;
			case Tkn_Identifier: {
				auto ident = parse_ident();
				return new ast::ValueExpr(ident);
			}
			case Tkn_IntLiteral: {
				auto token = current();
				advance();
				auto cty = ast::ConstantType::I32;
				if (check(Tkn_Identifier)) {
					auto value = token.ident->value->val;
					if(value == "i8")
						cty = ast::ConstantType::I8;
					else if(value == "i16")
						cty = ast::ConstantType::I16;
					else if(value == "i32")
						cty = ast::ConstantType::I32;
					else if(value == "i64")
						cty = ast::ConstantType::I64;
					else if(value == "u8")
						cty = ast::ConstantType::U8;
					else if(value == "u16")
						cty = ast::ConstantType::U16;
					else if(value == "u32")
						cty = ast::ConstantType::U32;
					else if(value == "u64")
						cty = ast::ConstantType::U64;
					else if (value == "f32")
						cty = ast::ConstantType::F32;
					else if (value == "f64")
						cty = ast::ConstantType::F64;
					else if(value == "char")
						cty = ast::ConstantType::Char;
				}
				return new ast::IntegerConstExpr(token.integer, cty, token.pos());
			} break;
			case Tkn_FloatLiteral: {
				auto token = current();
				advance();
				auto cty = ast::ConstantType::F32;
				if (check(Tkn_Identifier)) {
					auto value = token.ident->value->val;
					if (value == "f64")
						cty = ast::ConstantType::F64;
					else if(value == "i8")
						cty = ast::ConstantType::I8;
					else if(value == "i16")
						cty = ast::ConstantType::I16;
					else if(value == "i32")
						cty = ast::ConstantType::I32;
					else if(value == "i64")
						cty = ast::ConstantType::I64;
					else if(value == "u8")
						cty = ast::ConstantType::U8;
					else if(value == "u16")
						cty = ast::ConstantType::U16;
					else if(value == "u32")
						cty = ast::ConstantType::U32;
					else if(value == "u64")
						cty = ast::ConstantType::U64;
				}
				return new ast::FloatConstExpr(token.floating, cty, token.pos());
			} break;
			case Tkn_StringLiteral: {
				auto token = current();
				advance();
				return new ast::StringConstExpr(token.str, token.pos());
			} break;
			case Tkn_CharLiteral: {
				auto token = current();
				advance();
				return new ast::CharConstExpr(token.character, token.pos());
			} break;
			case Tkn_OpenBracket:
				break;
		}

		return nullptr;
	}

	ast::Expr* Parser::parse_block() {
		return nullptr;
	}

	ast::Expr* Parser::parse_suffix_expr(ast::Expr* already_parsed) {
		return already_parsed;
	}

	ast::Decl* Parser::parse_decl() { return nullptr; }
	ast::Decl* Parser::parse_toplevel_decl() { return nullptr; }
	ast::TypeSpec* Parser::parse_typespec() { return nullptr; }

	ast::Ident* Parser::parse_ident() {
		if(current().kind() != Tkn_Identifier) {
			expect(Tkn_Identifier);
			return nullptr;
		}
		auto token = current();
		advance();
		return token.ident;
	}

	mist::Token& Parser::peek() {
		return scanner->token();
	}

	mist::Token& Parser::current() { return curr; }

	void Parser::advance() {
		curr = scanner->token();
		scanner->advance();
		//std::cout << "Advancing: " << curr << std::endl;
	}

	bool Parser::one_of(std::vector<TokenKind> kind) {
		// build the list of tokens expected
		if(kind.empty()) {
			std::cout << "Empty One of Token list" << std::endl;
			return false;
		}
		std::string temp = "[";
		temp += mist::Token::get_string(kind[0]);

		for(auto iter = kind.begin() + 1; iter < kind.end(); ++iter) {
			temp += ", ";
			temp += mist::Token::get_string(*iter);
		}

		temp += "]";

		return one_of(kind, "Expecting one of: %s. Found: %s", temp.c_str(),
											current().get_string().c_str());
	}

	bool Parser::check(TokenKind kind) {
		return current().kind() == kind;
	}

	void Parser::expect(TokenKind kind) {
		expect(kind, "Expecting: %s. Found: %s", mist::Token::get_string(kind).c_str(),
							   current().get_string().c_str());
	}

	bool Parser::one_of(std::vector<TokenKind> kind, const std::string& msg, ...) {
		auto& t = current();
		auto elem = std::find(kind.begin(), kind.end(), t.kind());
		if (elem == kind.end()) {
			va_list va;
			const char* m = msg.c_str();
			va_start(va, m);
			interp->report_error(t.pos(), msg, va);
			va_end(va);

			return false;
		}
		return true;
	}



	void Parser::expect(TokenKind kind, const std::string& msg, ...) {
		auto& t = current();
		std::cout << "Expecting: " << t << std::endl;
		advance();
		if (t.kind() != kind) {
			va_list va;
			const char* m = msg.c_str();
			va_start(va, m);
		
			interp->report_error(t.pos(), msg, va);
	
			va_end(va);
		}
	}

	void Parser::sync() {

	}
}
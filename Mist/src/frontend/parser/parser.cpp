#include "parser.hpp"
#include <algorithm>

#include <cstdarg>
#include "interpreter.hpp"
#include "ast/ast_decl.hpp"
#include "ast/ast_expr.hpp"
#include "ast/ast_typespec.hpp"

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
		this->file = file;
		reset();


		auto module = new ast::Module(file);
		// while we are not at the end of the file. 
		// Try to parse a new declaration
		while(current().kind() != mist::Tkn_Eof) {
			auto d = parse_toplevel_decl();
			if(d)
				module->add_decl(d);
			else {
				interp->report_error(current().pos(), "failed to find top level declaration");
				sync();
			}
		}

		return module;
	}


	void Parser::reset() {
		// this needs to through an error
		if (!file) return;

		scanner->init(file);

		curr = scanner->token();
		scanner->advance();
	}

	ast::Expr* Parser::parse_expr() { return nullptr; }

	ast::Expr* Parser::parse_expr_with_res(i32 prec, Restriction res) {
		auto expr = parse_primary_expr();
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

	}

	ast::Expr* Parser::parse_bottom_expr() {

	}

	ast::Expr* Parser::parse_suffix_expr(ast::Expr* already_parsed) {

	}

	ast::Decl* Parser::parse_decl() { return nullptr; }
	ast::Decl* Parser::parse_toplevel_decl() { return nullptr; }
	ast::TypeSpec* Parser::parse_typespec() { return nullptr; }

	mist::Token& Parser::peek() {
		return scanner->token();
	}

	mist::Token& Parser::current() { return curr; }

	void Parser::advance() {
		curr = scanner->token();
		scanner->advance();
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
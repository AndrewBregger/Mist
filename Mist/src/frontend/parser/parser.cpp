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

		// auto e = parse_expr();

		// std::cout << e << std::endl;
		// ast::print(std::cout, e);

		auto module = new ast::Module(file);
		//// while we are not at the end of the file.
		//// Try to parse a new declaration
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
		
		if(check(Tkn_Comma) && ((res & StopAtComma) == 0)) {
			std::vector<ast::Expr*> lvalues = { expr };
			auto pos = expr->pos();
			while (check(mist::Tkn_Comma)) {
				pos = pos + current().pos();
				advance();
				lvalues.push_back(parse_expr_with_res(NoStructLiterals | StopAtComma));
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

			if(!token.is_operator() && !token.is_assignment()) {
				interp->report_error(current().pos(), "expecting binary or assignement assignment, found: %s", token.get_string().c_str());
			}

			auto rhs = parse_accoc_expr(curr_prec + 1);
			if(!rhs) return expr;
			auto pos = expr->pos() + token.pos() + rhs->pos();
			if(token.is_operator() && !token.is_assignment()) {
				if(expr->kind() == ast::Assignment) {
					interp->report_error(expr->pos(), "invalid sub expression of binary operator");
				}
				ast::BinaryOp op = (ast::BinaryOp) (token.kind() - mist::Tkn_Plus);
				expr = new ast::BinaryExpr(op, expr, rhs, pos);
			}
			else {
				ast::AssignmentOp op = (ast::AssignmentOp) (token.kind() - mist::Tkn_Equal);
				expr = new ast::AssignmentExpr(op, {expr}, rhs, pos);
			}
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

		// if there is no restrction agianst allowing in decl in this expression
		// then try to parse a decl. If there is on then this will return null
		// and continue parsing as normal.
		if ((res & NoDecl) == 0) {
			auto decl = try_parse_decl();
			if (decl)
				return decl;
		}

		auto c = current();
		switch(current().kind()) {
			case Tkn_Bang:
			case Tkn_Tilde:
			case Tkn_Minus:
			case Tkn_Astrick:
			case Tkn_Ampersand: {
				advance();
				auto e = parse_primary_expr();
				if(!e) return e;
				return new ast::UnaryExpr(ast::from_token(c.kind()), e, c.position + e->p);
			}
			default:
				return parse_atomic_expr();
		}
	}

	ast::Expr* Parser::parse_atomic_expr() {
		auto bottom = parse_bottom_expr();
		//ast::print(std::cout, bottom);
		return parse_suffix_expr(bottom);
	}

	ast::Expr* Parser::parse_bottom_expr() {
		auto token = current();
		std::cout << __FUNCTION__ << " " << token << std::endl;
		switch(token.kind()) {
			case Tkn_Unit: {
				advance();
				return new ast::UnitExpr(token.pos());
			}
			case Tkn_OpenParen: {
				advance();
				auto expr = parse_expr();
				if(!expr) return expr;
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
				return parse_value();
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
				return parse_block();
		}
		return nullptr;
	}

	ast::Expr* Parser::parse_block() {
		auto pos = current().pos();
		expect(Tkn_OpenBracket);

		/*
			this newline check is to allow there to be
			a new line following the opening of the
			block

			othewise this is the only block
			that is value: { x }

			now this is{
				x
			}
		*/
		if(check(Tkn_NewLine))
			advance();
		std::vector<ast::Expr*> elements;
		while(!check(Tkn_CloseBracket)) {
			if(check(Tkn_Eof)) {
				interp->report_error(current().pos(), "found end of file instead of '}'");
				return nullptr;
			}
			auto e = parse_expr();
			pos = pos + e->pos();
			elements.push_back(e);
			if(check(Tkn_NewLine)) {
				pos = pos + current().pos();
				advance();
			}
			else interp->report_error(current().pos(), "expecting new line at end of expression, found: %s", current().get_string().c_str());
		}
		pos = pos + current().pos();
		expect(Tkn_CloseBracket);

		return new ast::BlockExpr(elements, pos);
	}

	ast::Expr* Parser::parse_suffix_expr(ast::Expr* already_parsed) {
		std::cout << __FUNCTION__ << " " << current() << std::endl;
		auto token = current();
		auto expr = already_parsed;
		if(!expr) return nullptr;

		auto pos = expr->pos();
		bool running = true;
		while(running) {
			switch(current().kind()) {
				case Tkn_Period:
					pos = pos + current().pos();
					advance();
					expr = parse_dot_suffix(expr, pos);
					if(!expr) return nullptr;
					break;
				case Tkn_OpenParen:
					pos = pos + current().pos();
					advance();
					expr = parse_call(expr, pos);
					expect(Tkn_CloseParen, "expecting ')' following call");
					if(!expr) return nullptr;
					break;
				default:
					running = false;
			}
		}
		return expr;
	}

	ast::Expr* Parser::parse_dot_suffix(ast::Expr* operand, mist::Pos pos) {
		std::cout << __FUNCTION__ << " " << current() << std::endl;
		if(check(Tkn_Identifier)) {
			auto element = parse_value();
			if(!element) {
				interp->report_error(current().pos(), "expecting name following period, found: %s", current().get_string().c_str());
				return operand;
			}
			return new ast::SelectorExpr(operand, static_cast<ast::ValueExpr*>(element), pos + element->pos());
		}
		else if(check(Tkn_IntLiteral)) {
			auto token = current();
			advance();
			return new ast::TupleIndexExpr(operand, (i32) token.integer, pos + token.pos());
		}
		else {
			interp->report_error(current().pos(), "expecting an identifier or integer literal, found: %s", current().get_string().c_str());
			sync();
			return nullptr;
		}
	}

	ast::Expr* Parser::parse_call(ast::Expr* operand, mist::Pos pos) {
		std::vector<ast::Expr*> params;
		while (true) {
			if (check(Tkn_CloseParen)) break;

			if (check(Tkn_Identifier) && peek().kind() == Tkn_Colon) {
				auto name = current().ident;
				auto lpos = name->pos;
				advance();
				lpos = lpos + current().pos();
				expect(Tkn_Colon);
				auto expr = parse_expr_with_res(StopAtComma);
				if (!expr) {
					interp->report_error(current().pos(), "expecting expression in binding");
					sync();
					return operand;
				}
				lpos = lpos + expr->pos();
				pos = pos + lpos;
				params.push_back(new ast::BindingExpr(name, expr, lpos));
			}
			else {
				auto e = parse_expr_with_res(StopAtComma);
				if (!e) {
					interp->report_error(current().pos(), "expecting expression following comma");
					sync();
					return operand;
				}
				pos = pos + e->pos();
				params.push_back(e);
			}
			if (check(Tkn_Comma)) {
				advance();
			}
		}
		return new ast::ParenthesisExpr(operand, params, pos);
	}

	ast::Expr* Parser::parse_value() {
		auto ident = parse_ident();
		auto pos = ident->pos;
		std::vector<ast::Expr*> params;
		if(check(Tkn_OpenBrace)) {
			pos = pos + current().pos();
			advance();
			bool has_comma = false;
			while(!check(Tkn_CloseBrace) || has_comma) {
				has_comma = false;
				// the validity of theses expressions will be validated in the type checker.
				auto e = parse_expr_with_res(NoStructLiterals | StopAtComma);
				if(!e) {
					interp->report_error(current().pos(), "expecting expression in generic parameters");
					sync();
					return  nullptr;
				}
				pos = pos + e->pos();
				params.push_back(e);
				if(check(Tkn_Comma)) {
					has_comma = true;
					advance();
				}
				else
					break;
			}

			if(!check(Tkn_CloseBrace)) {
				if(current_can_begin_expression()) {
					interp->report_error(current().pos(), "expecting ',' between generics parameters, found: %s", current().get_string().c_str());
				}
			}
			expect(Tkn_CloseBrace);
		}
		return new ast::ValueExpr(ident, params, pos);
	}

	ast::Expr* Parser::try_parse_decl() {
		if(check_decl_from_expr()) {
			auto decl = parse_decl();
			return new ast::DeclExpr(decl);
		}
		return nullptr;
	}

	ast::Decl* Parser::parse_decl() {
		std::cout << "Parsing a decl" << std::endl;
		auto name = current();
		if(peek().kind() == Tkn_Comma) {
			std::vector<ast::ident*> names;
			auto pos = current().pos();
			names.push_back(current().ident);
			advance();
			while(check(tkn_comma)) {
				advance();
				if(check(tkn_identifier)) {
					names.push_back(current().ident);
					pos = pos + current().pos();
					advance();
				}
				else {
					interp->report_error(current().pos(), "expecting identifier following comma, found: '%s", current().get_string().c_str());
				}
			}
			return parse_local_decl(names, pos);
		}
		else {
			auto name = current().ident;
			expect(Tkn_Identifier);
			switch(current().kind()) {
				case Tkn_Colon:
				case Tkn_ColonEqual:
					return parse_local_decl({name}, name->pos);
				case Tkn_ColonColon: {
					advance();
					return parse_user_decl(name);
				}
			}
		}
		
		return nullptr;
	}

	ast::Decl* Parser::parse_local_decl(const std::vector<ast::Ident*>& names, mist::Pos pos) {
		std::vector<ast::TypeSpec*> specs;
		std::vector<ast::Expr*> exprs;
		ast::Expr* expr = nullptr;

		if(check(Tkn_Colon)) {
			advance();

			bool expectComma = false;	
			while(true) {
				if(check(Tkn_Equal) || check(Tkn_ColonEqual))
					break;
				expectComma = false;
				auto spec = parse_typespec();
				if(spec)
					specs.push_back(spec);
				else {
					interp->report_error(current().pos(), "expecting type specification following ','");
					return nullptr;
				}

				if(!check(Tkn_Comma))
					break;
				else {
					advance();
					expectComma = true;
				}
			}
		}

		if(check(Tkn_ColonEqual) || check(Tkn_Equal)) {
			advance();

			bool expectComma = false;	
			while(true) {
				if(check(Tkn_Equal) || check(Tkn_ColonEqual))
					break;
				expectComma = false;
				expr = parse_expr_with_res(StopAtComma);
				if(expr)
					exprs.push_back(expr);
				else {
					interp->report_error(current().pos(), "expecting expression following ','");
					return nullptr;
				}

				if(!check(Tkn_Comma))
					break;
				else {
					advance();
					expectComma = true;
				}
			}
		}
		
		if(names.size() > 1) {
			// this is for multiple local declarations
			return new ast::MultiLocalDecl(names, specs, exprs, pos);
		}
		else if(names.size() == 1) {
			if(specs.size() > 1) {
				interp->report_error(specs[1]->p, "expecting only one type specification following a single identifer");
			}

			if(exprs.size() > 1) {
				interp->report_error(exprs[1]->pos(), "expecting only one type specification following a single identifer");
			}
			return new ast::LocalDecl(names.front(), specs.front(), exprs.front(), pos);
		}

		interp->report_error(current().pos(), "expecting one of ':', ':=', '=' found: '%s'", current().get_string().c_str());
		return nullptr;
	}

	ast::Decl* Parser::parse_struct_decl(ast::Ident* name, ast::Generics* generics) {
		expect(Tkn_Struct);
		expect(Tkn_OpenBracket);
		if(check(Tkn_NewLine))
			advance();

		std::vector<ast::FieldDecl*> fields;
		while(!check(Tkn_CloseBracket)) {
			std::vector<ast::Ident*> names;
			auto pos = current().pos();
			names.push_back(current().ident);
			advance();`
			while(check(Tkn_Comma)) {
				advance();
				if(check(Tkn_Identifier)) {
					names.push_back(current().ident);
					pos = pos + current().pos();
					advance();
				}
				else {
					interp->report_error(current().pos(), "expecting identifier following comma, found: '%s", current().get_string().c_str());
				}
			}
			auto d = parse_local_decl(names, pos);
			if(d)
				fields.push_back(d);
			
			if()
		}
		return nullptr;
	}

	ast::Decl* Parser::parse_enum_decl(ast::Ident* name, ast::Generics* generics) {
		return nullptr;
	}

	ast::Decl* Parser::parse_typeclass_decl(ast::Ident* name, ast::Generics* generics) {
		return nullptr;
	}

	ast::Decl* Parser::parse_function_decl(ast::Ident* name, ast::Generics* generics) {
		return nullptr;
	}

	ast::Decl* Parser::parse_user_decl(ast::Ident* name) {
		// this will be null if it isnt found
		auto gen = parse_generics();
		switch(current().kind()) {
			case Tkn_Struct:
				return parse_struct_decl(name, gen);
			case Tkn_Enum:
				return parse_enum_decl(name, gen);
			case Tkn_OpenParen:
				return parse_function_decl(name, gen);
			case Tkn_Class:
				return parse_typeclass_decl(name, gen);
			default:
				one_of({Tkn_Struct, Tkn_Enum, Tkn_OpenParen, Tkn_Class});
		}
		return nullptr;
	}

	ast::Generics* Parser::parse_generics() {
		if(check(Tkn_OpenBrace)) {
			advance();
			auto g = parse_generic_decl();
			auto pos = g->pos;

			std::vector<ast::GenericDecl*> gens = { g };
			while(check(Tkn_Comma)) {
				advance();

				auto gen = parse_generic_decl();
				if(gen)
					gens.push_back(gen);
				else {
					interp->report_error(current().pos(), "expecting generic type declaration");
					break;
				}
			}

			expect(Tkn_CloseBrace);
			return new ast::Generics(gens);
		}
		return nullptr;
	}
	
	ast::GenericDecl* Parser::parse_generic_decl() {
		if(check(Tkn_Identifier)) {
			auto name = current().ident;
			auto pos = name->pos;
			advance();
			std::vector<ast::TypeSpec*> bounds;	
			if(check(Tkn_Colon)) {
				advance();
				while(true) {
					auto tspec = parse_typespec();
					if(tspec) {
						bounds.push_back(tspec);
						pos = pos + tspec->p;
					}
					else {
						interp->report_error(current().pos(), "expecting type following comma");
						break;
					}
					if(check(Tkn_Comma)) {
						advance();
					}
					else
						break;
				}
			}

			return new ast::GenericDecl(name, bounds, pos);
		}
		return nullptr;
	}

	ast::Decl* Parser::parse_toplevel_decl() {
		// add the parsing of use decls.
		auto decl = parse_decl();

		if(check(Tkn_NewLine)) {
			advance();
			return decl;
		}

		interp->report_error(current().pos(), "expecting newline following declaration, found: '%s'", current().get_string().c_str());
		return nullptr;
	}
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

	bool Parser::current_can_begin_expression() {
		switch(current().kind()) {
			case Tkn_Comma:
			case Tkn_Period:
			case Tkn_Colon:
			case Tkn_ColonEqual:
			case Tkn_ColonColon:
			case Tkn_Dollar:
			case Tkn_And:
			case Tkn_Or:
			case Tkn_Arrow:
			case Tkn_PeriodPeriod:
			case Tkn_At:
			case Tkn_Where:
			case Tkn_Derive:
			case Tkn_Struct:
			case Tkn_Enum:
			case Tkn_Else:
			case Tkn_Slash:
			case Tkn_Percent:
			case Tkn_AstrickAstrick:
			case Tkn_Less:
			case Tkn_Greater:
			case Tkn_LessEqual:
			case Tkn_GreaterEqual:
			case Tkn_EqualEqual:
			case Tkn_BangEqual:
			case Tkn_Equal:
			case Tkn_PlusEqual:
			case Tkn_MinusEqual:
			case Tkn_AstrickEqual:
			case Tkn_SlashEqual:
			case Tkn_PercentEqual:
			case Tkn_AstrickAstrickEqual:
			case Tkn_LessLessEqual:
			case Tkn_GreaterGreaterEqual:
			case Tkn_CarrotEqual:
			case Tkn_AmpersandEqual:
			case Tkn_PipeEqual:
			case Tkn_None:
				return false;
			default:
				break;
		}
		return true;
	}


	bool Parser::check_decl_from_expr() {
		Token token = current();

		// only check if there is an identifier there,
		// otherwise it isn't a declaration.
		if(check(Tkn_Identifier)) {
			Token p = peek();
			// this is a function, struct, enum declaration
			if(p.kind() == Tkn_ColonColon)
				return true;
			else if(p.kind() == Tkn_Comma) {
				auto oldState = save_state();
				while(check(Tkn_Identifier)) {
					advance();
					if(!check(Tkn_Comma))
						break;
					else
						advance();
				}
				if(check(Tkn_Colon) || check(Tkn_ColonEqual)) {
					restore_state(oldState);
					return true;
				}
				else {
					restore_state(oldState);
					return false;
				}
			}
			else if(p.kind() == Tkn_Colon) {
				return true;
			}
			else if(p.kind() == Tkn_ColonColon) {
				return true;
			}
			else if(p.kind() == Tkn_ColonEqual) {
				return true;
			}
			else return false;
		}
		// else if()
		return false;
	}

	void Parser::sync() {

	}

	Parser::SavedState Parser::save_state() {
		return SavedState {
			curr,
			res,
			scanner->save()
		};
	}

	void Parser::restore_state(const Parser::SavedState& state) {
		curr = state.current;
		res = state.res;
		scanner->restore(state.state);
	}
}
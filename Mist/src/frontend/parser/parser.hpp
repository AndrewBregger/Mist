#pragma once

#include "tokenizer/scanner.hpp"
#include "utils/file.hpp"
#include "ast/ast.hpp"


namespace ast {
	struct Expr;
	struct Decl;
	struct TypeSpec;
}

namespace mist {
	class Interpreter;
	typedef u32 Restriction;

	static Restriction Default = 0;
	static Restriction NoStructLiterals = 1;

	class Parser {
		public:
			Parser(mist::Interpreter* interp);

			// This is used when startin the parsing process.
			ast::Module* parse_root(io::File* file);

		private:

			// gets the parser ready for the new file.
			void reset();

			// I this is to be used by the Interpreter when I 
			// needs to parse module imports.
			ast::Module* parse_module(io::File* file);

			ast::Expr* parse_expr();

			ast::Expr* parse_expr_with_res(i32 prec, Restriction res = Default);

			ast::Expr* parse_primary_expr();

			ast::Expr* parse_atomic_expr();

			ast::Expr* parse_bottom_expr();

			ast::Expr* parse_suffix_expr(ast::Expr* already_parsed);

			ast::Decl* parse_decl();

			ast::Decl* parse_toplevel_decl();

			ast::TypeSpec* parse_typespec();

			mist::Token& peek();
			mist::Token& current();

			// advance the scanner to the next token and update the current.
			void advance();

			bool one_of(std::vector<TokenKind> kind);
			bool check(TokenKind kind);
			void expect(TokenKind kind);

			bool one_of(std::vector<TokenKind> kind, const std::string& msg, ...);
			void expect(TokenKind kind, const std::string& msg, ...);


			void sync();

			mist::Interpreter* interp; 	// interpreter
			io::File* file; 			// active file
			mist::Scanner* scanner; 	// scanner for this parser
			mist::Token curr;		// the current token. The scanner is one ahead.
			Restriction res = Default;

			friend class Interpreter;
	};
}
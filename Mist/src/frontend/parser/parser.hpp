#pragma once

#include "tokenizer/scanner.hpp"
#include "utils/file.hpp"
#include "ast/ast.hpp"


namespace ast {
	struct Expr;
	struct Decl;
	struct TypeSpec;
	struct Generics;
	struct GenericDecl;
	struct ValueExpr;
	struct LocalDecl;
	typedef LocalDecl FieldDecl;
}

namespace mist {
	class Interpreter;
	typedef u32 Restriction;

	static Restriction Default = 1 << 0;
	static Restriction IgnoreNewline = 1 << 1;
	static Restriction NoStructLiterals = 1 << 2;
	static Restriction StopAtComma = 1 << 3;
	static Restriction NoDecl = 1 << 4;
	static Restriction NoMultiSpecs = 1 << 5;

	class Parser {
		public:
			Parser(mist::Interpreter* interp);

			// This is used when startin the parsing process.
			ast::Module* parse_root(io::File* file);

		//private:

			// gets the parser ready for the new file.
			void reset();

			// I this is to be used by the Interpreter when I
			// needs to parse module imports.
			ast::Module* parse_module(io::File* file);

			ast::Expr* parse_expr();

			ast::Expr* parse_expr_with_res(Restriction res = Default);

			ast::Expr* parse_accoc_expr(i32 prec);

			ast::Expr* parse_primary_expr();

			ast::Expr* parse_atomic_expr();

			ast::Expr* parse_bottom_expr();

			ast::Expr* parse_suffix_expr(ast::Expr* already_parsed);

			ast::Expr* parse_dot_suffix(ast::Expr* operand, mist::Pos pos);

			ast::Expr* parse_call(ast::Expr* operand, mist::Pos pos);

			ast::Expr* parse_block();

			ast::Expr* parse_value();

			ast::Expr* try_parse_decl();

			bool check_decl_from_expr();

			ast::Decl* parse_decl();

			ast::Decl* parse_local_decl(const std::vector<ast::Ident*>& names, mist::Pos pos);

			ast::Decl* parse_struct_decl(ast::Ident* name, ast::Generics* generics);

			ast::Decl* parse_enum_decl(ast::Ident* name, ast::Generics* generics);

			ast::Decl* parse_typeclass_decl(ast::Ident* name, ast::Generics* generics);

			ast::Decl* parse_function_decl(ast::Ident* name, ast::Generics* generics);

			ast::Decl* parse_user_decl(ast::Ident* name);

			ast::Decl* parse_struct_decl_suffix(ast::Ident* name, ast::Generics* gens,
				const std::vector<ast::FieldDecl*>& fields);

			ast::WhereClause* parse_where_clause();

			std::vector<ast::TypeSpec*> parse_derive_suffix();

			ast::Generics* parse_generics();

			ast::GenericDecl* parse_generic_decl();

			ast::Decl* parse_toplevel_decl();

			ast::TypeSpec* parse_typespec();

			ast::Ident* parse_ident();

			mist::Token& peek();
			mist::Token& current();

			// advance the scanner to the next token and update the current.
			void advance();

			bool one_of(std::vector<TokenKind> kind);
			bool check(TokenKind kind);
			void expect(TokenKind kind);

			bool allow(TokenKind kind);

			template <typename... Args>
			bool one_of(std::vector<TokenKind> kind, const std::string& msg, Args... args) {
				auto& t = current();
				auto elem = std::find(kind.begin(), kind.end(), t.kind());
				if (elem == kind.end()) {
					interp->report_error(t.pos(), msg, args...);
					return false;
				}
				return true;
			}

			template <typename... Args>
			void expect(TokenKind kind, const std::string& msg, Args... args) {
				auto t = current();
				advance();
				if (t.kind() != kind)
					interp->report_error(t.pos(), msg, args...);
			}


			bool current_can_begin_expression();

			void sync();

			mist::Interpreter* interp; 	// interpreter
			io::File* file; 			// active file
			mist::Scanner* scanner; 	// scanner for this parser
			mist::Token curr;		// the current token. The scanner is one ahead.
			Restriction res = Default;

			struct SavedState {
				mist::Token current;
				Restriction res;
				mist::Scanner::State state;
			};


			SavedState save_state();
			void restore_state(const SavedState& state);

			ast::TypeSpec* value_to_type(ast::ValueExpr* expr);

			friend class Interpreter;
	};
}

#pragma once

#include <fstream>
#include "ast_common.hpp"
#include "ast_decl.hpp"
#include "ast_expr.hpp"
#include "ast_typespec.hpp"


namespace ast {
	void print(std::fstream& out, ast::Program* program);
	void print(std::fstream& out, Expr* expr);
	void print(std::fstream& out, Decl* decl);
	void print(std::fstream& out, TypeSpec* spec);
}


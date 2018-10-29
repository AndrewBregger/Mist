#pragma once

#include <fstream>
#include "ast_common.hpp"
#include "ast_decl.hpp"
#include "ast_expr.hpp"
#include "ast_typespec.hpp"


namespace ast {
	void print(std::fstream& out, ast::Program* program);
	std::ostream& print(std::ostream& out, Expr* expr);
	std::ostream& print(std::ostream& out, Decl* decl);
	std::ostream& print(std::ostream& out, TypeSpec* spec);
}


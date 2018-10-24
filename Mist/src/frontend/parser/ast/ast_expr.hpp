#pragma once

#include "ast_common.hpp"

namespace ast {
	enum ExprKind {
		// Represents an identifier.
		Value,

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
		Decl,
		Parenthesis,
		Selector,
		

	};
}
#include "ast_printer.hpp"

namespace ast {
	void print(std::fstream& out, ast::Program* program) {
		for(auto decl : program->toplevelDeclarations)
			print(out, decl);
	}

	void print(std::fstream& out, Expr* expr) {
		if(!expr) return;
		switch (expr->k) {
			case Value:
				break;
			case Tuple:
				break;
			case IntegerConst:
				break;
			case FloatConst:
				break;
			case StringConst:
				break;
			case BooleanConst:
				break;
			case CharConst:
				break;
			case Binary:
				break;
			case Unary:
				break;
			case If:
				break;
			case While:
				break;
			case Loop:
				break;
			case For:
				break;
			case Match:
				break;
			case DeclDecl:
				break;
			case Parenthesis:
				break;
			case Selector:
				break;
			case Break:
				break;
			case Continue:
				break;
			case Return:
				break;
			case Cast:
				break;
			case Range:
				break;
			case Slice:
				break;
			case TupleIndex:
				break;
			case Assignment:
				break;
			case Block:
				break;
		}
	}

	void print(std::fstream& out, Decl* decl) {
		if(!decl) return;
		switch(decl->k) {
			case Local:
				break;
			case MultiLocal:
				break;
			case Struct:
				break;
			case TypeClass:
				break;
			case Function:
				break;
			case Use:
				break;
			case Impl:
				break;
			case Generic:
				break;
		}
	}

	void print(std::fstream& out, TypeSpec* spec) {
		if(!spec) return;
		switch(spec->k) {
			case Named:
				break;
			case TupleType:
				break;
			case FunctionType:
				break;
			case TypeClassType:
				break;
			case Array:
				break;
			case DynamicArray:
				break;
			case Map:
				break;
			case Pointer:
				break;
			case Reference:
				break;
			case Constant:
				break;
			case Path:
				break;
		}
	}
}
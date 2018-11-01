#include "ast_printer.hpp"
#include "interpreter.hpp"
#include "frontend/parser/tokenizer/token.hpp"

#define CAST(T, e) static_cast<T*>(e)

#define PRINT(l) { \
	for(auto x : l) \
		print(out, x); \
}

namespace ast {
	void print(std::ostream& out, ast::Module* program) {
		out << "Program: [" << std::endl;
		for(auto decl : program->toplevelDeclarations)
			print(out, decl);

		out << "]" << std::endl;
	}

	std::ostream& print(std::ostream& out, Expr* expr) {
		if(!expr) return out;
		out << expr->name() << ": {" << std::endl;
		out << "pos: { line: " << expr->p.line << ", column: " << expr->p.column << ", span: " << expr->p.span << " }," << std::endl;
		switch (expr->k) {
			case Value: {
				auto e = CAST(ValueExpr, expr);
					out << "name: " << e->name->value->val << std::endl;
				if(!e->genericValues.empty()) {
					out << "params: [" << std::endl;
					PRINT(e->genericValues)
					out << "], " << std::endl;
				}
			} break;
			case Tuple: {
				auto e = CAST(TupleExpr, expr);
				out << "values: [" << std::endl;
				PRINT(e->values)
				out << "], " << std::endl;
			} break;
			case IntegerConst: {
				auto e = CAST(IntegerConstExpr, expr);
				out << "value: " << e->value << std::endl;
			} break;
			case FloatConst: {
				auto e = CAST(FloatConstExpr, expr);
				out << "value: " << e->value << std::endl;
			} break;
			case StringConst: {
				auto e = CAST(StringConstExpr, expr);
				out << "value: " << e->value << std::endl;
			} break;
			case BooleanConst: {
				auto e = CAST(BooleanConstExpr, expr);
				out << "value: " << (e->value ? "true" : "false") << std::endl;
			} break;
			case CharConst: {
				auto e = CAST(CharConstExpr, expr);
				out << "value: " << e->value << std::endl;
			} break;
			case Binary: {
				auto e = CAST(BinaryExpr, expr);
				out << "op: " << mist::Token::get_string((mist::TokenKind) (mist::Tkn_Plus + e->op)) << ", " << std::endl;
				out << "lhs: {" << std::endl;
				print(out, e->lhs) << "}," << std::endl;
				out << "rhs: {" << std::endl;
				print(out, e->rhs) << "}" << std::endl;
				break;
			}
			case Unary: {
				auto e = CAST(UnaryExpr, expr);
				out << "op: " << mist::Token::get_string(ast::from_unary(e->op)) << ", " << std::endl;
				out << "value: {" << std::endl;
				print(out, e->expr) << "}" << std::endl;
			} break;
			case If: {
				auto e = CAST(IfExpr, expr);
				out << "cond: {" << std::endl;
				print(out, e->cond) << std::endl << "}," << std::endl;
				out << "body: {" << std::endl;
				print(out, e->body) << std::endl << "}" << std::endl;
				break;
			}
			case While: {
				auto e = CAST(WhileExpr, expr);
				out << "cond: {" << std::endl;
				print(out, e->cond) << std::endl << "}," << std::endl;
				out << "body: {" << std::endl;
				print(out, e->body) << std::endl << "}" << std::endl;
				break;
			}
			case Loop: {
				auto e = CAST(LoopExpr, expr);
				out << "body: {" << std::endl;
				print(out, e->body) << std::endl << "}" << std::endl;
				break;
			}
			case For: {
				auto e = CAST(ForExpr, expr);
				out << "index: {" << std::endl;
				print(out, e->index) << std::endl << "}," << std::endl;
				out << "expr: {" << std::endl;
				print(out, e->expr) << std::endl << "}," << std::endl;
				out << "body: {" << std::endl;
				print(out, e->body) << std::endl << "}" << std::endl;
				break;
			}
			case Match: {
				break;
			}
			case DeclDecl: {
				auto e = CAST(DeclExpr, expr);
				out << "decl: {" << std::endl;
				print(out, e->decl) << std::endl << "}" << std::endl;
				break;
			}
			case Parenthesis: {
				auto e = CAST(ParenthesisExpr, expr);
				out << "operand: {" << std::endl;
				print(out, e->operand) << std::endl << "}," << std::endl;
				out << "params: {" << std::endl;
				PRINT(e->params)
				out << "}" << std::endl;
				break;
			}
			case Selector: {
				auto e = CAST(SelectorExpr, expr);
				out << "operand: {" << std::endl;
				print(out, e->operand) << "}," << std::endl;
				out << "element: {" << std::endl;
				print(out, e->element) << "}" << std::endl;
				break;
			}
			case Break:
			case Continue:
				break;
			case Return: {
				auto e = CAST(ReturnExpr, expr);
				if(!e->returns.empty()) {
					out << "returns: {" << std::endl;
					PRINT(e->returns);
					out << std::endl << "}" << std::endl;
				}
				break;
			}
			case Cast: {
				auto e = CAST(CastExpr, expr);
				out << "expr: {" << std::endl;
				print(out, e->expr) << std::endl << "}," << std::endl;
				out << "type: {" << std::endl;
				print(out, e->ty) << std::endl << "}" << std::endl;
				break;
			}
			case Range: {
				auto e = CAST(RangeExpr, expr);
				out << "low: {" << std::endl;
				print(out, e->low) << std::endl << "}," << std::endl;
				out << "high: {" << std::endl;
				print(out, e->high) << std::endl << "}" << std::endl;
				out << "count: {" << std::endl;
				print(out, e->count) << std::endl << "}" << std::endl;
				break;
			}
			case Slice: {
				auto e = CAST(SliceExpr, expr);
				out << "low: {" << std::endl;
				print(out, e->low) << std::endl << "}," << std::endl;
				out << "high: {" << std::endl;
				print(out, e->high) << std::endl << "}" << std::endl;
				break;
			}
			case TupleIndex: {
				auto e = CAST(TupleIndexExpr, expr);
				out << "operand: {" << std::endl;
				print(out, e->operand) << std::endl << "}," << std::endl;
				out << "high: " << e->index << "," << std::endl;
				break;
			}
			case Assignment: {
				auto e = CAST(AssignmentExpr, expr);
				out << "op: " << mist::Token::get_string((mist::TokenKind) (mist::Tkn_Equal + e->op)) << ",\n";
				out << "lvalues: {" << std::endl;
				PRINT(e->lvalues)
				out << std::endl << "}," << std::endl;
				out << "expr: {" << std::endl;
				print(out, e->expr) << std::endl << "}" << std::endl;
				break;
			}
			case Block: {
				auto e = CAST(BlockExpr, expr);
				out << "elements: {" << std::endl;
				PRINT(e->elements)
				out << std::endl << "}" << std::endl;
				break;
			}
		}
		out << "}," << std::endl;
		return out;
	}

	std::ostream& print(std::ostream& out, Decl* decl) {
		if(!decl) return out;
		out << decl->string() << ": {" << std::endl;
		out << "pos: { line: " << decl->pos.line << ", column: " << decl->pos.column << ", span: " << decl->pos.span << " }," << std::endl;
		if(decl->k != MultiLocal) {
			out << "name: " << decl->name->value->val << "," << std::endl;
		}
		switch(decl->k) {
			case Local: {
				auto d = CAST(LocalDecl, decl);
				if(d->sp) {
					out << "type: {" << std::endl;
					print(out, d->sp) << std::endl << "}," << std::endl;
				}
				if(d->init) {
					out << "init: {" << std::endl;
					print(out, d->init) << std::endl << "}," << std::endl;
				}
				break;
			}
			case MultiLocal: {
				auto d = CAST(MultiLocalDecl, decl);
				out << "names: {" << std::endl;
				for(auto x : d->names) {
					out << x->value->val << "," << std::endl;
				}
				out << "}," << std::endl;
				out << std::endl << "}," << std::endl;
				if(!d->sps.empty()) {
					out << "types: {" << std::endl;
					PRINT(d->sps);
					out << std::endl << "}," << std::endl;
				}
				if(!d->inits.empty()) {
					out << "inits: {" << std::endl;
					PRINT(d->inits);
					out << std::endl << "}," << std::endl;
				}
				break;
			}
			case Struct: {
				break;
			}
			case TypeClass: {
				break;
			}
			case Function: {
				break;
			}
			case Use: {
				break;
			}
			case Impl: {
				break;
			}
			case Generic: {
				break;
			}
		}
		out << "}," << std::endl;
		return out;
	}

	std::ostream& print(std::ostream& out, TypeSpec* spec) {
		if(!spec) return out;
//		out << spec->name() << ": {" << std::endl;
//		out << "pos: { line: " << spec->p.line << ", column: " << spec->p.column << ", span: " << spec->p.span << " }," << std::endl;
		switch(spec->k) {
			case Named: {
				out << CAST(NamedSpec, spec)->name->value->val;
				break;
			}
			case TupleType: {
				out << "(";
				PRINT(CAST(TupleSpec, spec)->types)
				out << ")";
				break;
			}
			case FunctionType: {
				auto s = CAST(FunctionSpec, spec);
				out << "(";
				print(out, s->parameters[0]);
				for(auto iter = s->parameters.begin() + 1; iter < s->parameters.end(); ++iter) {
					out << ',';
					print(out, *iter);
				}
				out << ") -> ";
				print(out, s->returns[0]);
				for(auto iter = s->returns.begin() + 1; iter < s->returns.end(); ++iter) {
					out << ',';
					print(out, *iter);
				}

				break;
			}
			case TypeClassType: {
				print(out, CAST(TypeClassSpec, spec)->name);
				break;
			}
			case Array: {
				auto s = CAST(ArraySpec, spec);
				out << '[' << s->size->value << ']';
				print(out, s->element);
				break;
			}
			case DynamicArray: {
				auto s = CAST(DynamicArraySpec, spec);
				out << "[..]";
				print(out, s->element);
				break;
			}
			case Map: {
				auto s = CAST(MapSpec, spec);
				out << "[ ";
				print(out, s->key);
				out << ", ";
				print(out, s->value);
				out << " ]";
				break;
			}
			case Pointer: {
				out << "*";
				print(out, spec->base);
				break;
			}
			case Reference: {
				out << "&";
				print(out, spec->base);
				break;
			}
			case Constant: {
				out << "const ";
				print(out, spec->base);
				break;
			}
			case Path: {
				const auto& p = CAST(PathSpec, spec)->path;
				print(out, p[0]);
				for(auto iter = p.begin() + 1; iter < p.end(); ++iter) {
					out << '.';
					print(out, *iter);
				}
				break;
			}
			case Unit: {
				out << "Unit";
			} break;
		}
		return out;
	}
}
//
// Created by Andrew Bregger on 2018-12-18.
//

#pragma once

#include "ast/ast.hpp"
#include "ast/ast_expr.hpp"
#include "ast/ast_decl.hpp"
#include "ast/ast_typespec.hpp"
#include "interpreter.hpp"
#include "tokenizer/token.hpp"
#include "tokenizer/scanner.hpp"

#define PDEBUG() PDEBUG_(__FUNCTION__)
#define PDEBUG_(fn) std::cout << fn << " " << current() << std::endl
namespace mist {
    class Parser {
    public:

        typedef u64 Restriction;
        const Restriction NoStructLiteral = 1 << 0;

        /// Parser constructor
        /// @param interp takes the interpreter for resource management.
        Parser(Interpreter *interp);

        /// Parses the root file
        /// @param root the root file. This file must contain main.
        /// @return the turns a module ast node
        ast::Module* parse_root(io::File* root);

        ast::Expr* parse_expr();

    private:
        /// parses and expression with an expected restriction.
        ast::Expr* parse_expr_with_res(Restriction res);

        /// parses an associative expression.
        ast::Expr* parse_assoc_expr(int min_prec);

        /// parses a unary expression
        ast::Expr* parse_unary();

        /// parses the a primary expression.
        ///     functions and dot expressions.
        ast::Expr *parse_primary(ast::Expr *operand);

        /// parses a basic expression.
        /// The simpilest of the expressions.
        /// identifiers, control flow, and checks for declarations.
        ast::Expr* parse_bottom();


        ast::Expr* parse_suffix(ast::Expr* opernad);

        ast::Expr* parse_call(ast::Expr* operand);

        ast::Expr* parse_lambda();

        /// consumes an identifier.
        ast::Ident* parse_ident();

        /// consumes an identifer and checks for generics following (later).
        ast::ValueExpr* parse_value();

        /// consumes an identifer following a literal
        /// if it matches an expected value
        ast::ConstantType parse_literal_suffix();

    private:

        template <typename AstNode, typename ParseFn>
        std::vector<AstNode*> many(ParseFn fn, TokenKind delim, bool allow_trailing = false) {
                std::vector<AstNode*> res;
                do {
                        auto node = fn();
                        if(node)
                            res.push_back(node);
                        else if(!allow_trailing) {
                                // this handles the emtpy case
                                if(res.empty())
                                        return res;
                                interp->report_error(current().pos(),
                                        "execting expression following '%s'", mist::Token::get_string(delim).c_str());
                                break;
                        }
                } while(allow(delim));
                return res;
        }

        /// Gets the current token
        Token current();

        /// Gets the next tokenkind from the scanner
        TokenKind peak();

        /// Advances the scanner to the next token
        void advance();

        /// Implementation of a single advance
        void advance_impl();


        /// Checks whether the current token is expected
        bool check(mist::TokenKind k);


        /// Token k must be the current token. Errors otherwise.
        bool expect(mist::TokenKind k);

        /// Checks if the current token is k. Consumes it.
        bool allow(mist::TokenKind k);

    private:
        mist::Interpreter* interp{nullptr};    /// Interpreter pointer
        Scanner* scanner{nullptr};             /// Scanner used by the parser
        Token prev;                            /// The previous valid token (not comment)
        Token curr;                            /// Current token
        bool ignore_comments{false};           /// Tells the parser to ignore
///                                            ///      whitespace or to process it
        Restriction restriction;               /// Current restrictions when parsing
    };
}

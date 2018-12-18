//
// Created by Andrew Bregger on 2018-12-18.
//

#pragma once

#include "interpreter.hpp"
#include "tokenizer/token.hpp"
#include "tokenizer/scanner.hpp"

namespace mist {
    class Parser {
    public:
        /// Parser constructor
        /// @param interp takes the interpreter for resource management.
        Parser(Interpreter *interp);

        /// Parses the root file
        /// @param root the root file. This file must contain main.
        /// @return the turns a module ast node
        ast::Module* parse_root(io::File* root);


    private:
        /// Gets the current token
        Token current();

        /// Gets the next token from the scanner
        Token peak();

        /// Advances the scanner to the next token
        void advance();


        /// Implementation of a single advance
        void advance_impl();

        mist::Interpreter* interp{nullptr};    /// Interpreter pointer
        Scanner* scanner{nullptr};             /// Scanner used by the parser
        Token curr;                            /// Current token
        bool ignore_comments{false};           /// Tells the parser to ignore
///                                            ///      whitespace or to process it
    };
}

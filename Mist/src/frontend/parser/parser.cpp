//
// Created by Andrew Bregger on 2018-12-18.
//

#include "parser.hpp"

mist::Parser::Parser(mist::Interpreter *interp) : interp(interp) {}

ast::Module *mist::Parser::parse_root(io::File *root) {
    return nullptr;
}

mist::Token mist::Parser::current() {
    return curr;
}

mist::Token mist::Parser::peak() {
    return scanner->token();
}


void mist::Parser::advance_impl() {
    curr = scanner->token();
    scanner->advance();
}

void mist::Parser::advance() {
    if(ignore_comments) {
        while(current().kind == Tkn_Comment)
            advance_impl();
    }
    else
        advance_impl();
}

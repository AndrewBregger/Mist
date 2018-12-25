//
// Created by Andrew Bregger on 2018-12-18.
//

#include "parser.hpp"

mist::Parser::Parser(mist::Interpreter *interp) : interp(interp),
    scanner(new mist::Scanner(interp)) {}

ast::Module *mist::Parser::parse_root(io::File *root) {
    ignore_comments = true;

    scanner->init(root);


    if(interp->has_error())
        return nullptr;

    scanner->advance();
    advance_impl();

    auto module = new ast::Module(root);


    // process imports
    return module;
}

ast::Expr *mist::Parser::parse_expr() {
    return parse_expr_with_res(0);
}

ast::Expr *mist::Parser::parse_expr_with_res(mist::Parser::Restriction res) {
    PDEBUG();
    auto old = restriction;
    restriction = res;
    auto expr = parse_assoc_expr(1);
    restriction = old;
    return expr;
}

ast::Expr *mist::Parser::parse_assoc_expr(int min_prec) {
    PDEBUG();
    auto expr = parse_unary();

    while(current().prec() >= min_prec) {
        auto token = current();

        auto curr_prec = token.prec();

        if (curr_prec < min_prec)
            break;

        advance();
        auto rhs = parse_assoc_expr(curr_prec + 1);
        if(!rhs) {
            interp->report_error(current().pos(), "expecting primary expression");
            return expr;
        }

        auto pos = expr->pos() + token.pos() + rhs->pos();
        if(token.is_operator() && !token.is_assignment()) {
            if(expr->kind() == ast::Assignment) {
                interp->report_error(expr->pos(), "assignment as sub expression");
                break;
            }

            ast::BinaryOp op = (ast::BinaryOp) (token.kind() - mist::Tkn_Plus);
            expr = new ast::BinaryExpr(op, expr, rhs, pos);
        }
        else {
            auto op = (ast::AssignmentOp) (token.kind() - mist::Tkn_Plus);
            expr = new ast::AssignmentExpr(op, {expr}, rhs, pos);
        }
    }

    return expr;
}

ast::Expr *mist::Parser::parse_unary() {
    PDEBUG();
    auto token = current();
    switch(token.kind()) {
        case Tkn_Ampersand:
        case Tkn_Astrick:
        case Tkn_Minus:
        case Tkn_Bang:
        case Tkn_Tilde: {
            ast::UnaryOp op = ast::from_token(token.kind());
            advance();
            auto expr = parse_unary();
            return new ast::UnaryExpr(op, expr, const_cast<Pos&>(token.pos()) + expr->pos());
        }
        default:
            break;
    }
    auto operand = parse_bottom();
    return parse_primary(operand);
}

ast::Expr *mist::Parser::parse_primary(ast::Expr *operand) {
    PDEBUG();
    while(true) {
        auto token = current();
        switch (token.kind()) {
            case Tkn_Colon: {
                // bind expression
                advance();

                auto expr = parse_expr();
                if(!expr) {
                    interp->report_error(current().pos(), "expecting epxression following ':'");
                }
                operand = new ast::BindingExpr(operand, expr,
                        operand->pos() + token.pos() + expr->pos());
            } continue;
            case Tkn_OpenBracket: {
                // struct expression
                advance();
                auto pos = operand->pos();
                auto members = many<ast::Expr>([this]() {
                    return this->parse_expr();
                }, Tkn_Comma, true);
                expect(Tkn_CloseBracket);

                for(auto x : members)
                    pos = pos + x->pos();

                operand = new ast::StructLiteralExpr(operand,
                        members, pos);
            } continue;
            case Tkn_Period:
                operand = parse_suffix(operand);
                continue;
            case Tkn_OpenParen:
                operand = parse_call(operand);
                continue;
            default:
                break;
        }
        break;
    }
    return operand;
}

ast::Expr *mist::Parser::parse_bottom() {
    PDEBUG();
    auto token = current();
    switch(token.kind()) {
        case Tkn_Identifier:
            return parse_value();
        case Tkn_IntLiteral: {
            advance();
            auto suf = parse_literal_suffix();

            if (suf != ast::ConstantType::None)
                advance();

            return new ast::IntegerConstExpr(token.integer, suf,
                                             (suf == ast::ConstantType::None ? token.pos() :
                                              const_cast<mist::Pos &>(token.pos()) + current().pos()));
        }
        case Tkn_FloatLiteral: {
            advance();
            auto suf = parse_literal_suffix();

            if (suf != ast::ConstantType::None)
                advance();

            return new ast::FloatConstExpr(token.floating, suf,
                                           (suf == ast::ConstantType::None ? token.pos() :
                                            const_cast<mist::Pos &>(token.pos()) + current().pos()));
        }
        case Tkn_CharLiteral: {
            advance();
            return new ast::CharConstExpr(token.character, token.pos());
        }
        case Tkn_True: {
            advance();
            return new ast::BooleanConstExpr(true, token.pos());
        }
        case Tkn_False: {
            advance();
            return new ast::BooleanConstExpr(false, token.pos());
        }
        case Tkn_Break:
            advance();
            return new ast::BreakExpr(token.pos());
        case Tkn_Continue:
            advance();
            return new ast::ContinueExpr(token.pos());
        case Tkn_Defer:
            interp->report_error(current().pos(), "defer expressions are not implemented at the momented");
            break;
        case Tkn_SelfLit:
            advance();
            return new ast::SelfExpr(token.pos());
        case Tkn_Unit:
            advance();
            return new ast::UnitExpr(token.pos());
        case Tkn_OpenParen: {
            advance();
            // tuple, or (<expr>)
            auto elements = many<ast::Expr>([this]() {
                return this->parse_expr();
            }, Tkn_Comma, false);
            auto pos = token.pos();

            for (auto e : elements)
                pos = pos + e->pos();

            pos = pos + current().pos();
            expect(Tkn_CloseParen);
            if (elements.size() == 1)
                return elements.front();
            else {
                return new ast::TupleExpr(elements, pos);
            }
        }
        case Tkn_BackSlash:
            // anonomous function
            return parse_lambda();
        case Tkn_OpenBrace: {
            auto pos = token.pos();
            advance();
            if (check(Tkn_CloseBrace)) {
                // list/dictionary comprehension
                if (peak() == Tkn_For || peak() == Tkn_While) {
                    interp->report_error(current().pos(), "list comprehension is not implemented yet");
                }
            }
            // array or dictionary literal
            auto elements = many<ast::Expr>([this]() {
                return this->parse_expr();
            }, Tkn_Comma, true);

            for (auto x : elements)
                pos = pos + x->pos();

            pos = pos + current().pos();

            expect(Tkn_CloseBrace);

            return new ast::CompoundLiteralExpr(elements, pos);
        }
        case Tkn_OpenBracket: {
            auto pos = current().pos();
            advance();
            auto elements = many<ast::Expr>([this]() {
                return this->parse_expr();
            }, Tkn_Semicolon, true);

            for (auto e : elements)
                pos = pos + e->pos();


            // I don't think this is right place to handle expression lists.
            // But it is the only place that we will have all of the information
            // to insert this expression.


            // the definition of an expression list in a block is that
            // a semicolon is used to seperate expressions. However, if
            // there is a trailing semicolon at the end of the list, an unit expression
            // is append at the end so that the expressions returns the unit value.
            if(prev.kind() == Tkn_Semicolon)
                elements.push_back(new ast::UnitExpr(prev.pos()));

            pos = pos + current().pos();
            expect(Tkn_CloseBracket);
            return new ast::BlockExpr(elements, pos);
        }

        case Tkn_If:
        case Tkn_For:
        case Tkn_While:
        case Tkn_Loop:
        default:
            break;
    }
    return nullptr;
}

ast::Expr *mist::Parser::parse_suffix(ast::Expr *operand) {
    expect(Tkn_Period);
    auto token = current();
    switch(token.kind()) {
        case Tkn_Identifier: {
            auto ident = parse_value();
            return new ast::SelectorExpr(operand, ident, operand->pos() + ident->pos());
        }
        case Tkn_IntLiteral:  {
            advance();
            return new ast::TupleIndexExpr(operand, token.integer, operand->pos() + token.pos());
        }
        default:
            interp->report_error(current().pos(), "expecting identifier or integer literal following period");
    }
    return nullptr;
}

ast::Expr *mist::Parser::parse_call(ast::Expr *operand) {
    expect(Tkn_OpenParen);
    auto pos = operand->pos();
    auto params = many<ast::Expr>([this] () {
            return this->parse_expr();
        }, Tkn_Comma);
    expect(Tkn_CloseParen);
    for(auto e : params)
        pos = pos + e->pos();
    return new ast::ParenthesisExpr(operand, params, pos);
}

ast::Ident *mist::Parser::parse_ident() {
    auto token = current();
    if(allow(Tkn_Identifier)) {
        return token.ident;
    }
    else
        return nullptr;

}

ast::Expr *mist::Parser::parse_lambda() {
    return nullptr;
}

ast::ValueExpr *mist::Parser::parse_value() {
    auto ident = parse_ident();
    // parse identifier
    return new ast::ValueExpr(ident, std::vector<ast::Expr*>(), ident->pos);
}

ast::ConstantType mist::Parser::parse_literal_suffix() {
    auto value = current().str;
    ast::ConstantType cty = ast::ConstantType::None;
    if (value == "f64")
        cty = ast::ConstantType::F64;
    else if(value == "i8")
        cty = ast::ConstantType::I8;
    else if(value == "i16")
        cty = ast::ConstantType::I16;
    else if(value == "i32")
        cty = ast::ConstantType::I32;
    else if(value == "i64")
        cty = ast::ConstantType::I64;
    else if(value == "u8")
        cty = ast::ConstantType::U8;
    else if(value == "u16")
        cty = ast::ConstantType::U16;
    else if(value == "u32")
        cty = ast::ConstantType::U32;
    else if(value == "u64")
        cty = ast::ConstantType::U64;
    return cty;
}

mist::Token mist::Parser::current() {
    return curr;
}

mist::TokenKind mist::Parser::peak() {
    return scanner->token().kind();
}

void mist::Parser::advance_impl() {
    curr = scanner->token();
    scanner->advance();
}

bool mist::Parser::check(mist::TokenKind k) {
   return current().kind() == k;
}

bool mist::Parser::expect(mist::TokenKind k) {
    if(check(k)) {
        advance();
        return true;
    }
    else {
        interp->report_error(current().pos(), "expected: '%s', found: '%s'",
                mist::Token::get_string(k).c_str(),
                current().get_string().c_str());
        return false;
    }
}

bool mist::Parser::allow(mist::TokenKind k) {
    if(check(k)) {
        advance();
        return true;
    }
    return false;
}

void mist::Parser::advance() {
    if(ignore_comments) {
        // the current token will never be a comment
        auto local = current();
        do {
            advance_impl();
        } while(current().kind() == Tkn_Comment);
        // set the previous to the last valid token
        prev = local;
    }
    else
        advance_impl();
}

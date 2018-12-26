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
    restriction |= res;
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
                if(restriction & NoStructLiteral) {
                    if(!(restriction & NoError))
                        interp->report_error(current().pos(), "structure literal is invalid here");
                    return operand;
                }
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


ast::Pattern *mist::Parser::parse_pattern() {
    PDEBUG();
    auto token = current();
    switch(token.kind()) {
        case Tkn_Identifier:
        case Tkn_OpenParen:
        case Tkn_Underscore:
            return parse_lvalue_pattern();
        case Tkn_OpenBrace:
            return parse_range_pattern();
        case Tkn_Minus: {
            advance();
            auto lit = current();
            expect(Tkn_IntLiteral);
            return new ast::IntegerPat(-token.integer, const_cast<Pos&>(lit.pos()) + const_cast<Pos&>(token.pos()));
        }
        case Tkn_IntLiteral: {
            advance();
            return new ast::IntegerPat(token.integer, token.pos());
        }
        case Tkn_FloatLiteral:
            advance();
            return new ast::FloatPat(token.floating, token.pos());
        case Tkn_StringLiteral: {
           auto str = interp->find_string(token.str);
           return new ast::StringPat(str, token.pos());
        }
        case Tkn_CharLiteral:
            advance();
            return new ast::CharacterPat(token.character, token.pos());
        case Tkn_True:
            advance();
            return new ast::BooleanPat(true, token.pos());
        case Tkn_False:
            advance();
            return new ast::BooleanPat(false, token.pos());
        default:
            break;
    }
    return nullptr;
}

ast::Pattern *mist::Parser::parse_lvalue_pattern() {
    PDEBUG();
    auto token = current();

    switch(token.kind()) {
        case Tkn_Identifier: {
            // this is a name or a selection, which mean it has to be
            // a struct/variant pattern
            switch(peak()) {
                case Tkn_Period:
                case Tkn_OpenBrace:
                case Tkn_OpenParen:
                case Tkn_OpenBracket:
                    return parse_struct_variant_pattern(parse_expr_with_res(NoStructLiteral | NoError));
                case Tkn_Underscore:
                    advance();
                    return new ast::UnderscorePat(token.pos());
                default: {
                    auto ident = parse_ident();
                    return new ast::IdentPat(ident, ident->pos);
                }
            }
        }
        case Tkn_Underscore: {
            advance();
            return new ast::UnderscorePat(token.pos());
        }
        case Tkn_OpenParen: {
            advance();
            auto pos = token.pos();
            auto elements = many<ast::Pattern>([this] () {
                return this->parse_pattern();
            }, Tkn_Comma);
            expect(Tkn_CloseParen);
            for(auto e : elements)
                pos = pos + e->pos();
            return new ast::TuplePat(elements, pos);
        }
        default:
            break;
    }
    return nullptr;
}

ast::Pattern *mist::Parser::parse_struct_variant_pattern(ast::Expr *name) {
    PDEBUG();
    auto token = current();
    auto opposite = token.kind();
    switch(token.kind()) {
        case Tkn_OpenParen:
            opposite = Tkn_CloseParen;
            break;
        case Tkn_OpenBracket:
            opposite = Tkn_CloseBracket;
            break;
        default:
            interp->report_error(current().pos(), "expecting '[' or '}'");
    }

    advance();

    auto members = many<ast::Pattern>([this] () {
        return this->parse_pattern();
    }, Tkn_Comma);

    auto pos = name->pos() + token.pos();
    for(auto m : members)
        pos = pos + m->pos();

    pos = pos + current().pos();
    expect(opposite);

    if(token.kind() == Tkn_OpenParen)
        return new ast::VariantPat(name, members, pos);
    else
        return new ast::StructPat(name, members, pos);
}

ast::Pattern *mist::Parser::parse_range_pattern() {
    PDEBUG();
    expect(Tkn_OpenBrace);
    auto pos = current().pos();
    auto low = parse_expr_with_res(NoStructLiteral);
    auto token = current();
    bool inclusive = false;

    switch(token.kind()) {
        case Tkn_PeriodPeriod:
            inclusive = false;
            break;
        case Tkn_PeriodPeriodPeriod:
            inclusive = true;
            break;
        default:
            interp->report_error(token.pos(), "expecting '..' or '...' found: '%s'", token.get_string().c_str());
            break;
    }
    advance();
    auto high = parse_expr_with_res(NoStructLiteral);
    pos = pos + low->pos() + token.pos() + high->pos() + const_cast<Pos&>(token.pos());
    expect(Tkn_CloseBrace);

    return new ast::RangePat(low, high, inclusive, pos);
}

ast::Decl *mist::Parser::parse_decl() {
    if(check(Tkn_Use)) {
        return parse_use();
    }
    else {
        auto pat = parse_pattern();
        if(check(Tkn_Colon)) {
            return parse_local(pat);
        }
        else {
            if(expect(Tkn_ColonColon)) {
                return parse_type_decl(pat);
            }
        }
    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_structure(ast::Pattern *name) {
    if(name->kind() != ast::PatternKind::IdentPatKind) {
        interp->report_error(name->pos(), "invalid structure name");
        // sync();
        return nullptr;
    }

    if(expect(Tkn_Struct)) {
        auto members = many<ast::Decl>([this]() {
            auto pat = this->parse_pattern();
            return this->parse_local(pat);
        }, Tkn_Comma, true);

        expect(Tkn_CloseBracket);

        ast::WhereClause* where = parse_where();
    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_class(ast::Pattern *name) {
    if(name->kind() != ast::PatternKind::IdentPatKind) {
        interp->report_error(name->pos(), "invalid class name");
        // sync();
        return nullptr;
    }
    if(expect(Tkn_Class)) {

    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_impl(ast::Pattern *name) {
    if(name->kind() != ast::PatternKind::IdentPatKind) {
        interp->report_error(name->pos(), "invalid invalid type name for impl block");
        // sync();
        return nullptr;
    }
    if(expect(Tkn_Impl)) {

    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_variant(ast::Pattern *name) {
    if(name->kind() != ast::PatternKind::IdentPatKind) {
        interp->report_error(name->pos(), "invalid variant name");
        // sync();
        return nullptr;
    }
    if(expect(Tkn_Variant)) {

    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_function(ast::Pattern *name) {
    if(name->kind() != ast::PatternKind::IdentPatKind) {
        interp->report_error(name->pos(), "invalid function name");
        // sync();
        return nullptr;
    }
    if(expect(Tkn_OpenParen)) {

    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_type_decl(ast::Pattern *name) {
    // auto gen = parse_generics_decl();
    auto token = current();
    switch(token.kind()) {
        case Tkn_Struct:
            return parse_structure(name);
        case Tkn_Class:
            return parse_class(name);
        case Tkn_Impl:
            return parse_impl(name);
        case Tkn_Variant:
            return parse_variant(name);
        case Tkn_OpenParen:
            return parse_function(name);
        default:
            interp->report_error(token.pos(), "expecting: 'struct', 'class', 'impl', or 'variant' found: '%s'",
                    token.get_string().c_str());
            break;
    }
    if(expect(Tkn_Struct)) {

    }
    return nullptr;
}

ast::Decl *mist::Parser::parse_local(ast::Pattern *name) {
    return nullptr;
}

ast::Decl *mist::Parser::parse_use() {
    return nullptr;
}

ast::TypeSpec *mist::Parser::parse_typespec() {

    auto mut = ast::Mutablity::Immutable;

    if(allow(Tkn_Mut))
        mut = ast::Mutablity::Mutable;
    auto token = current();
    switch(token.kind()) {
        case Tkn_Identifier:
            return parse_path(mut);
        case Tkn_OpenParen:
            return parse_function_or_tuple(mut);
        case Tkn_OpenBrace: {
            ast::TypeSpec* key = nullptr;
            ast::Expr* size = nullptr;
            auto pos = current().pos();
            advance();

            auto mutkey = ast::Mutablity::Immutable;
            if(check(Tkn_Mut)) {
                mut = ast::Mutablity::Mutable;
                key = parse_typespec();
            } else {
                size = parse_expr();
                if(!check(Tkn_Comma)) {
                    /// array
                    if(expect(Tkn_CloseBrace)) {
                        auto type = parse_typespec();
                        return new ast::ArraySpec(type, size, mut, pos);
                    }
                    return nullptr;
                }
                else {
                    /// map
                    key = expr_to_typespec(mutkey, size);
                }
            }
            auto comma = current();
            if(expect(Tkn_Comma)) {
                auto value = parse_typespec();
                pos = pos + key->pos() + value->pos() + comma.pos() + current().pos();
                expect(Tkn_CloseBrace);
                return new ast::MapSpec(key, value, mut, pos);
            }
            return nullptr;
        }
        case Tkn_Astrick: {
            advance();
            auto type = parse_typespec();
            return new ast::PointerSpec(type, mut, const_cast<Pos &>(token.pos()) + type->pos());
        }
        case Tkn_Ref:
        case Tkn_Ampersand:{
            advance();
            auto type = parse_typespec();
            return new ast::ReferenceSpec(type, mut, const_cast<Pos&>(token.pos()) + type->pos());
        }
        case Tkn_Unit:
            advance();
            return new ast::UnitSpec(token.pos());
        default:
            break;
    }
    return nullptr;
}

ast::TypeSpec * mist::Parser::parse_function_or_tuple(ast::Mutablity mut) {
    // parse a tuple
    auto pos = current().pos();
    auto fn = [this] () {
        return this->parse_typespec();
    };
    if(expect(Tkn_OpenParen)) {
        auto elements = many<ast::TypeSpec>(fn, Tkn_Comma);
        for(auto e : elements)
            pos = pos + e->pos();
        expect(Tkn_CloseParen);

        if(allow(Tkn_MinusGreater)) {
            auto rets = many<ast::TypeSpec>(fn, Tkn_Comma);

            for(auto e : rets)
                pos = pos + e->pos();

            return new ast::FunctionSpec(elements, rets, pos);
        }
        else
            return new ast::TupleSpec(elements, mut, pos);
    }
    return nullptr;
}

ast::TypeSpec *mist::Parser::parse_path(ast::Mutablity mut) {
    auto parse_name = [this, mut] () {
        auto ident = this->parse_ident();
        // handle generics
        return new ast::NamedSpec(ident, nullptr, mut, ident->pos);
    };

    std::vector<ast::NamedSpec*> names;
    auto pos = current().pos();
    pos.span = 0;
    do {
        auto name = parse_name();
        if(name) {
            names.push_back(name);
            pos = pos + name->pos();
        }
        else {
            interp->report_error(current().pos(), "expecting identifier, found: '%s'", current().get_string().c_str());
            // sync();
            break;
        }
    } while(allow(Tkn_Period));

    return new ast::PathSpec(names, mut, pos);
}


bool reduce(std::vector<ast::NamedSpec*>& names, ast::Expr* curr) {
    switch(curr->kind()) {
        case ast::Selector: {
            auto s = static_cast<ast::SelectorExpr*>(curr);
            auto r = reduce(names, s->operand);
            if(r) {
                return reduce(names, s->element);
            } else
                return false;

        } break;
        case ast::Value: {
            auto v = static_cast<ast::ValueExpr*>(curr);
            // auto gen = convert_generics(v->genericValues);
            auto name = new ast::NamedSpec(v->name, nullptr, ast::Mutable, v->name->pos);
            names.push_back(name);
            return true;
        }
        default:
            return false;
    }
}

ast::TypeSpec *mist::Parser::expr_to_typespec(ast::Mutablity mut, ast::Expr *expr) {
    std::vector<ast::NamedSpec*> names;

    if(reduce(names, expr)) {
        if(names.empty()) {
            return nullptr;
        }

        auto pos = names.front()->pos();
        pos.span = 0;

        for(auto x : names)
            pos = pos + x->pos();

        return new ast::PathSpec(names, mut, pos);
    }
    return nullptr;
}

ast::WhereClause *mist::Parser::parse_where() {
    auto parse_where_element = [this] () -> ast::WhereElement* {
        auto name = parse_ident();
        if(expect(Tkn_Colon)) {
            auto pos = name->pos;
            auto bounds = many<ast::TypeSpec>([this]() {
                return parse_path(ast::Immutable);
            }, Tkn_Plus);

            for(auto b : bounds)
                pos = pos + b->pos();
            return new ast::WhereElement(name, bounds, pos);
        }
        else
            return nullptr;
    };

    if(expect(Tkn_Where)) {
        auto pos = prev.pos();

        auto elements = many<ast::WhereElement>(parse_where_element, Tkn_Comma);

        for(auto p : elements)
            pos = pos + p->pos;

        return new ast::WhereClause(elements, pos);
    }

    return nullptr;
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

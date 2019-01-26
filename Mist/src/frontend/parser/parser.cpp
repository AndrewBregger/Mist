//
// Created by Andrew Bregger on 2018-12-18.
//

#include "parser.hpp"

mist::Parser::Parser(Typer* typer, mist::Interpreter *interp) : interp(interp),
    scanner(new mist::Scanner(interp)), typer(typer) {}

ast::Module *mist::Parser::parse_root(io::File *root) {
    ignore_comments = true;

    scanner->init(root);


    if(interp->has_error())
        return nullptr;

    scanner->advance();
    advance_impl();

    auto module = new ast::Module(root);
//    module->scope = typer->get_current();

    while(!check(Tkn_Eof)) {
        auto decl = parse_decl(true);
        if(decl) {
            if (decl->kind() == ast::Local) {
                auto pattern = static_cast<ast::LocalDecl*>(decl)->name;

                if(pattern->kind() != ast::IdentPatKind)
                    interp->report_error(pattern->pos(), "invalid local pattern in gloabl file scope");
            }

            module->add_decl(decl);
        }
        else
            interp->report_error(current().pos(), "unable to find a valid declaration");
    }

    // process imports
    return module;
}

mist::Typer* mist::Parser::get_typer() {
    return typer;
}

ast::Expr *mist::Parser::parse_expr() {
    return parse_expr_with_res(0);
}

ast::Expr *mist::Parser::parse_expr_with_res(mist::Parser::Restriction res) {
    PDEBUG();
    auto old = restriction;
    add_restriction(res);
    auto expr = parse_assoc_expr(1);
    restriction = old;
    return expr;
}

ast::Expr *mist::Parser::parse_assoc_expr(int min_prec) {
    PDEBUG();
    auto expr = parse_unary();
    if(!expr)
        return nullptr;

    // if we are parsing the lhs of an expression then do not continue if a comma is found.

    auto old = restriction;
    add_restriction(RhsExpression);

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
            auto op = (ast::AssignmentOp) (token.kind() - mist::Tkn_Equal);
            expr = new ast::AssignmentExpr(op, {expr}, rhs, pos);
        }
    }
    restriction = old;

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
                ast::Expr* expr = nullptr;

                // bind expression
                advance();

                ast::Mutability  mut = ast::Immutable;
                if(allow(Tkn_Mut))
                    mut = ast::Mutable;

                expr = parse_expr();

                if(allow(Tkn_Equal)) {
                    ast::Ident* ident = nullptr;
                    if(operand->kind() == ast::Value) {
                        // @TODO: Add checking for generics when that is added.
                        ident = static_cast<ast::ValueExpr*>(operand)->name;
                        delete operand;
                    }
                    else {
                        // i think this is an error.
                        break;
                    }
                    // this is actually a local
                    auto init = parse_expr_with_res(RhsExpression);
                    ast::TypeSpec* type = nullptr;
                    if(expr)
                        type = expr_to_typespec(mut, expr);

                    std::vector<ast::TypeSpec*> specs;
                    std::vector<ast::Expr*> inits;
                    auto pos = ident->pos;

                    if(type) {
                        specs.push_back(type);
                        pos = pos + type->pos();
                    }

                    if(init) {
                        inits.push_back(init);
                        pos = pos + init->pos();
                    }

                    auto decl = new ast::LocalDecl(
                                new ast::IdentPat(ident, ident->pos),
                                specs, inits, pos
                            );

                    return new ast::DeclExpr(decl);
                }

                if(!(restriction & AllowBindExpressions))
                    break;

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
                auto old = restriction;
                add_restriction(AllowBindExpressions);
                auto members = many<ast::Expr>([this]() {
                    return this->parse_expr();
                }, Tkn_Comma, true);
                restriction = old;
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
        // this is to allow import into a specific scope instead of populating the entire file scope.
        case Tkn_Pub:
            return new ast::DeclExpr(parse_decl());
        case Tkn_Use:
            return new ast::DeclExpr(parse_decl());
        case Tkn_Identifier:
            if(peak() == Tkn_Comma) {
                if(!(restriction & RhsExpression)) {
                    auto decl = parse_decl();
                    return new ast::DeclExpr(decl);
                }
            }
            else if(peak() == Tkn_Colon || peak() == Tkn_ColonColon) {
                // we found a decl.
                auto decl = parse_decl();
                return new ast::DeclExpr(decl);
            }

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
//            auto scope = typer->push_scope(BlockScope);
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
            auto block = new ast::BlockExpr(elements, pos);
//            block->scope = scope;
//            typer->pop_scope();
            return block;
        }

        case Tkn_If:
            advance();
            return parse_if();
        case Tkn_For:
            advance();
            return parse_for();
        case Tkn_While:
            advance();
            return parse_while();
        case Tkn_Loop:
            return parse_loop();
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
    auto old = restriction;
    add_restriction(AllowBindExpressions);
    auto params = many<ast::Expr>([this] () {
            return this->parse_expr();
        }, Tkn_Comma);
    restriction = old;
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

ast::Expr *mist::Parser::parse_if() {
    auto pos = prev.pos();

    auto cond = parse_expr_with_res(RhsExpression);

    auto expr = parse_expr();

    if(!expr) {
        interp->report_error(current().pos(), "expecting expression in if body");
        // sync();
    }

    ast::Expr* elif = nullptr;

    if(allow(Tkn_Elif))
        elif = parse_if();
    else if(allow(Tkn_Else))
        elif = parse_expr();
    pos = pos + cond->pos() + expr->pos();
    if(elif) {
        pos = pos + elif->pos();
    }

    return new ast::IfExpr(cond, expr, elif, pos);
}

ast::Expr *mist::Parser::parse_for() {
    auto pos = prev.pos();

    auto pat = parse_pattern();

    auto con = parse_expr_with_res(RhsExpression);

    auto expr = parse_expr();
    pos = pos + pat->pos() + con->pos() + expr->pos();
    return new ast::ForExpr(pat, con, expr, pos);
}

ast::Expr *mist::Parser::parse_while() {
    auto pos = prev.pos();

    auto cond = parse_expr_with_res(RhsExpression);
    auto expr = parse_expr();
    pos = pos + cond->pos() + expr->pos();
    auto wexpr = new ast::WhileExpr(cond, expr, pos);
    return wexpr;

}

ast::Expr *mist::Parser::parse_loop() {
    auto pos = prev.pos();
    auto expr = parse_expr_with_res(RhsExpression);
    pos = pos + expr->pos();
    auto lexpr = new ast::LoopExpr(expr, pos);
    return lexpr;
}

ast::ValueExpr *mist::Parser::parse_value() {
    auto ident = parse_ident();
    // parse identifier
    return new ast::ValueExpr(ident, std::vector<ast::Expr*>(), ident->pos);
}

ast::Pattern *mist::Parser::parse_pattern() {
    auto pattern = parse_pattern_main();

    if(restriction & LocalAsField) {
        return pattern;
    }

    if(check(Tkn_Comma) || check(Tkn_Pipe)) {
        auto pos = pattern->pos();
        
        auto res = std::vector<ast::Pattern*>({pattern});
        auto delim = current();
        advance();

        auto pats = many<ast::Pattern>([this] () {
            return this->parse_pattern_main();
        }, delim.kind());

        for(auto p : pats) {
            res.push_back(p);
            pos = pos + p->pos();
        }

        for(auto p : pats) {
            if(p->kind() == ast::StructurePatKind)
                interp->report_error(p->pos(), "unable to have structure pattern in compound patterns");
        }
        if(delim.kind() == Tkn_Comma)
            return new ast::ListPat(res, pos);
        else 
            return nullptr;
    }
    return pattern;
}

ast::Pattern *mist::Parser::parse_pattern_main() {
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

ast::Decl *mist::Parser::parse_decl(bool toplevel_decls) {
    bool set_visible = false;
    if(allow(Tkn_Pub))
        set_visible = true;
    ast::Decl* decl = nullptr;
    if(check(Tkn_Use)) {
        decl = parse_use();
    }
    else {
        auto pat = parse_pattern();
        if(check(Tkn_Colon)) {
            if(toplevel_decls) {
                decl = parse_global(pat);
            }
            else {
                decl = parse_local(pat);
            }

        }
        else {
            if(expect(Tkn_ColonColon)) {
                decl = parse_type_decl(pat);
            }
        }
    }

    if(decl) {
        if(set_visible) {
            decl->vis = ast::Visibility::Public;
            return decl;
        }
        else
            return decl;
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
        std::vector<ast::LocalDecl*> members;
        expect(Tkn_OpenBracket);
        if(!check(Tkn_CloseBracket)) {
            auto old = restriction;
            add_restriction(LocalAsField);
            members = many<ast::LocalDecl>([this]() {
                auto pat = this->parse_pattern();
                return (ast::LocalDecl*) this->parse_local(pat);
            }, Tkn_Comma, true);
            restriction = old;
        }
        expect(Tkn_CloseBracket);


        ast::WhereClause* where = nullptr;
        if(check(Tkn_Where))
            where = parse_where();
        /// parse derives
        std::vector<ast::TypeSpec*> derives;
        if(check(Tkn_Derive)) {
            // do derives things
        }

		auto ident = static_cast<ast::IdentPat*>(name)->name;
		delete name;

		auto pos = name->pos();
		for(auto m : members)
			pos = pos + m->pos;
		for(auto d : derives)
			pos = pos + d->pos();

        return new ast::StructDecl(ident, members, derives, where, nullptr, pos);
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
        std::vector<ast::TypeSpec*> parents;
        std::vector<ast::LocalDecl*> members;

        if(!check(Tkn_OpenBracket)) {
            parents = many<ast::TypeSpec>([this] () {
                return this->parse_path(ast::Immutable);
            }, Tkn_Comma);
        }

        expect(Tkn_OpenBracket);
        if(!check(Tkn_CloseBracket)) {
            auto old = restriction;
            add_restriction(LocalAsField);
            members = many<ast::LocalDecl>([this]() {
                auto pat = this->parse_pattern();
                return (ast::LocalDecl*) this->parse_local(pat);
            }, Tkn_Comma, true);
            restriction = old;
        }

        expect(Tkn_CloseBracket);


        ast::WhereClause* where = nullptr;
        if(check(Tkn_Where))
            where = parse_where();
        /// parse derives
        std::vector<ast::TypeSpec*> derives;
        if(check(Tkn_Derive)) {
            // do derives things
        }

        auto ident = static_cast<ast::IdentPat*>(name)->name;
        delete name;

        auto pos = name->pos();
        for(auto m : members)
            pos = pos + m->pos;
        for(auto d : derives)
            pos = pos + d->pos();

        return new ast::ClassDecl(ident, members, derives, where, nullptr, parents, pos);
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

ast::VariantMemberDecl *mist::Parser::parse_variant_member() {
    auto ident = parse_ident();
    ast::Expr* init = nullptr;
    std::vector<ast::TypeSpec*> members;

    if(allow(Tkn_OpenParen)) {
        auto pos = ident->pos;
        members = many<ast::TypeSpec>([this] () {
            return this->parse_typespec();
        }, Tkn_Comma);
        expect(Tkn_CloseParen);

        for(auto x : members)
            pos = pos + x->pos();

        if(check(Tkn_Equal))
            interp->report_error(current().pos(), "unable to initialize a variant structure");

        return new ast::VariantMemberDecl(ident, ast::VariantStruct, pos, members, nullptr);
    }
    else {
        auto pos = ident->pos;
        if(allow(Tkn_Equal)) {
            init = parse_expr_with_res(RhsExpression);
            pos = pos + init->pos();
        }

        return new ast::VariantMemberDecl(ident, ast::VariantIdent, pos);
    }
}


ast::Decl *mist::Parser::parse_variant(ast::Pattern *name) {
    if(name->kind() != ast::PatternKind::IdentPatKind) {
        interp->report_error(name->pos(), "invalid variant name");
        // sync();
        return nullptr;
    }
    auto ident = static_cast<ast::IdentPat*>(name)->name;
    if(expect(Tkn_Variant)) {
        auto pos = name->pos();
        expect(Tkn_OpenBracket);
        auto members = many<ast::VariantMemberDecl>([this] () {
            return this->parse_variant_member();
        }, Tkn_Comma, true);
        expect(Tkn_CloseBracket);

//        auto bodyScope = typer->push_scope(BlockScope);

        for(auto x : members) {
            pos = pos + x->pos;

            typer->add_name(x->name, x);
        }

//        typer->pop_scope();

        auto vdecl = new ast::VariantDecl(ident, members, nullptr, pos);
//        typer->add_pattern(name, vdecl);
//        vdecl->scope = bodyScope;
        return vdecl;
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
        auto old = restriction;
        std::vector<ast::TypeSpec*> rets;
        std::vector<ast::FieldDecl*> fields;
        auto pos = name->pos();

        add_restriction(LocalAsField);
//        auto paramscope = typer->push_scope(ParamScope);

        if(!check(Tkn_CloseParen))
            fields = many<ast::LocalDecl>([this] () {
                auto pat = this->parse_pattern();
                return (ast::LocalDecl*) this->parse_local(pat);
            }, Tkn_Comma);


        for(auto p : fields)
            pos = pos + p->pos;

        expect(Tkn_CloseParen);

        auto token = current();

        if(allow(Tkn_MinusGreater)) {
            pos = pos + token.pos();
            rets = many<ast::TypeSpec>([this] () {
                return this->parse_typespec();
            }, Tkn_Comma);
            for(auto p : rets)
                pos = pos + p->pos();
        }

        if(rets.empty())
            rets.push_back(new ast::UnitSpec(mist::Pos()));

        if(!allow(Tkn_Equal)) {
            expect(Tkn_OpenParen);
        }
        auto body = parse_expr();



//        auto bodyscope = body->get_scope();


        auto fdecl = new ast::FunctionDecl(static_cast<ast::IdentPat*>(name)->name, fields, rets, body, nullptr, pos);
//        fdecl->paramScope = paramscope;
//        fdecl->bodyScope = bodyscope;

//        typer->pop_scope(); // this will pop the parameters scope off.
//        typer->add_pattern(name, fdecl);
        return fdecl;
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
    std::vector<ast::TypeSpec*> spec;
    std::vector<ast::Expr*> init;


    if(restriction & LocalAsField) {
        ast::Mutability mut = ast::Immutable;
        if(allow(Tkn_Mut))
            mut = ast::Mutable;

        auto token = current();
        if(allow(Tkn_SelfLit)) {
            if(name)
                interp->report_error(name->pos(), "self following a valid name");
            return new ast::SelfFieldDecl(mut, token.pos());
        }

        if(mut == ast::Mutable) {
            interp->report_error(token.pos(), "expecting 'self'");
            // sync();
            return nullptr;
        }
    }

    auto pos = name->pos();
    if(allow(Tkn_Colon)) {
        if(!(restriction & LocalAsField))
            spec = many<ast::TypeSpec>([this]() {
                return this->parse_typespec();
            }, Tkn_Comma);
        else {
            auto t = parse_typespec();
            if(t)
                spec.push_back(t);
        }

        for(auto s : spec)
            pos = pos + s->pos();

        check_typespec_and_pattern(name, spec);

        if(allow(Tkn_Equal)) {
            if(!(restriction & LocalAsField))
                init = many<ast::Expr>([this] () {
                    return this->parse_expr_with_res(RhsExpression);
                }, Tkn_Comma);
            else
                init.push_back(parse_expr_with_res(RhsExpression));

            for(auto s : init)
                pos = pos + s->pos();
        }
    }
    else if(!(restriction & LocalAsField)) {
        interp->report_error(current().pos(), "expecting ':' in local declaration");
        return nullptr;
    }
    auto ldecl = new ast::LocalDecl(name, spec, init, pos);
//    typer->add_pattern(name, ldecl);
    return ldecl;
}

ast::Decl *mist::Parser::parse_global(ast::Pattern *name) {

    ast::TypeSpec* spec = nullptr;
    ast::Expr* init = nullptr;

    if(name->kind() != ast::IdentPatKind) {
        interp->report_error(name->pos(), "global variabibles must have a single name, invalid global name pattern");
    }

    auto pos = name->pos();
    if(allow(Tkn_Colon)) {
        if(!check(Tkn_Equal)) {
            spec = parse_typespec();
            if(!spec) {
                interp->report_error(current().pos(), "expecting one of '*', 'ref', '(', or identifier following ':'");
                // sync();
            }
            else {
                pos = pos + spec->pos();
            }
        }

        if(allow(Tkn_Equal)) {
            init = this->parse_expr_with_res(RhsExpression);
            if(init) {
                pos = pos + init->pos();
            }
            else {
                interp->report_error(current().pos(), "expecting expression follwowing '='");
                // sync();
            }
        }
    }

    if(!spec && !init) {
        interp->report_error(current().pos(), "must have a type or initializing expression on global");
        // sync();
    }

    auto pat = static_cast<ast::IdentPat*>(name);
    auto n = pat->name;

    return new ast::GlobalDecl(n, spec, init, pos);
}

ast::Decl *mist::Parser::parse_use() {
    return nullptr;
}

ast::TypeSpec *mist::Parser::parse_typespec() {

    auto mut = ast::Mutability::Immutable;

    if(allow(Tkn_Mut))
        mut = ast::Mutability::Mutable;
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

            auto mutkey = ast::Mutability::Immutable;
            if(check(Tkn_Mut)) {
                mut = ast::Mutability::Mutable;
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

ast::TypeSpec * mist::Parser::parse_function_or_tuple(ast::Mutability mut) {
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

ast::TypeSpec *mist::Parser::parse_path(ast::Mutability mut) {
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

bool mist::Parser::check_typespec_and_pattern(ast::Pattern *pat, const std::vector<ast::TypeSpec *> &spec, bool subPattern) {
    // there is nothing to check in this case.
    if(spec.size() == 0)
        return true;

    if(pat->kind() == ast::StructurePatKind ||
       pat->kind() == ast::VariantPatKind) {
        if (spec.size() != 0) {
            interp->report_error(pat->pos(), "type annotations are not compatable with pattern");
            return false;
        }

        if(subPattern) {
            interp->report_error(pat->pos(), "invalid sub-pattern");
            return false;
        }
    }

    switch(pat->kind()) {
        case ast::IntegerLiteralPatKind:
        case ast::FloatLiteralPatKind:
        case ast::StringLiteralPatKind:
        case ast::CharLiteralPatKind:
        case ast::BooleanPatKind:
        case ast::RangePatKind:
            interp->report_error(pat->pos(), "invalid pattern in variable declaration");
            return false;
        case ast::OptionalPatKind: {
//            auto p = static_cast<ast::Pa
        }
        case ast::ListPatKind: {
            auto p = static_cast<ast::ListPat*>(pat);
            if(p->patterns.size() != spec.size()) {
                interp->report_error(p->pos(), "expecting/ '%u' type annoations found: '%u'", p->patterns.size(), spec.size());
            }
            for(u32 i = 0; i < p->patterns.size(); ++i)
                if(!check_typespec_and_pattern(p->patterns[i], {spec[i]}, true))
                    return false;
        }
    }
    return true;
}

ast::TypeSpec *mist::Parser::expr_to_typespec(ast::Mutability mut, ast::Expr *expr) {
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
    ast::ConstantType cty = ast::ConstantType::None;
    if(!check(Tkn_Identifier))
        return cty;
    auto value = current().ident->value->value();
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
    else if(value == "f32")
        cty = ast::ConstantType::F32;
    else if(value == "f64")
        cty = ast::ConstantType::F64;
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

//
// Created by Andrew Bregger on 2018-12-24.
//

#include "ast_pattern.hpp"

ast::Pattern::Pattern(ast::PatternKind k, mist::Pos p) : k(k), p(p) {

}

ast::IdentPat::IdentPat(ast::Ident *name, mist::Pos pos) : Pattern(IdentPatKind, pos), name(name) {

}

ast::TuplePat::TuplePat(const std::vector<ast::Pattern *> &elements, mist::Pos pos) :
    Pattern(TuplePatKind, pos), elements(elements){
}

ast::StructPat::StructPat(ast::Expr *name, const std::vector<ast::Pattern *> &members, mist::Pos pos) :
    Pattern(StructurePatKind, pos), name(name), members(members) {
}

ast::VariantPat::VariantPat(ast::Expr *name, const std::vector<ast::Pattern *> &members, mist::Pos pos) :
    Pattern(VariantPatKind, pos), name(name), members(members) {
}

ast::IntegerPat::IntegerPat(i64 value, mist::Pos pos) : Pattern(IntegerLiteralPatKind, pos), value(value) {
}

ast::FloatPat::FloatPat(float value, mist::Pos pos) : Pattern(FloatLiteralPatKind, pos), value(value) {
}

ast::StringPat::StringPat(mist::String *value, mist::Pos pos) : Pattern(StringLiteralPatKind, pos), value(value) {
}

ast::CharacterPat::CharacterPat(char value, mist::Pos pos) : Pattern(CharLiteralPatKind, pos), value(value) {
}

ast::BooleanPat::BooleanPat(bool value, mist::Pos pos) : Pattern(BooleanPatKind, pos), value(value) {
}

ast::RangePat::RangePat(ast::Expr *low, ast::Expr *high, mist::Pos pos) : Pattern(RangePatKind, pos),
    low(low), high(high){
}



#include "token.hpp"
#include <vector>

static const std::vector<std::string> token_strings = {
#define TOKEN_KIND(n, str) str,
    TOKEN_KINDS
#undef TOKEN_KIND
};

namespace mist {

#define TOKEN_CONSTRUCTOR_IMP(Type, Elem, TokenType) \
    TOKEN_CONSTRUCTOR_DEF(Type) : Token(TokenType, pos) { \
        this->Elem = elem;  \
    }

	Token::Token() = default;

	Token(TokenKind kind, const Pos& pos) : tokenKind(kind), position(pos) {}

    TOKEN_CONSTRUCTOR_IMP(u64, integer, Tkn_IntLiteral)
    TOKEN_CONSTRUCTOR_IMP(f64, floating, Tkn_FloatLiteral)
    TOKEN_CONSTRUCTOR_IMP(char, character, Tkn_CharLiteral)
    TOKEN_CONSTRUCTOR_IMP(const std::string&, str, Tkn_StringLiteral)
    TOKEN_CONSTRUCTOR_IMP(ast::Ident*, ident, Tkn_Identifier);


    Token::~Token() {}

    const std::string& Token::get_string(TokenKind kind) {
        return token_strings[kind];
    }
}

#include "token.hpp"
#include <vector>
#include <algorithm>

static const std::vector<std::string> token_strings = {
#define TOKEN_KIND(n, str) str,
    TOKEN_KINDS
#undef TOKEN_KIND
};

namespace mist {

#define TOKEN_CONSTRUCTOR_IMP(Type, Elem, TokenType) \
    Token::TOKEN_CONSTRUCTOR_DEF(Type) : Token(TokenType, pos) { \
        this->Elem = elem;  \
    }

	Token::Token() {}

	Token::Token(TokenKind kind, const Pos& pos) : tokenKind(kind), position(pos) {}

    TOKEN_CONSTRUCTOR_IMP(u64, integer, Tkn_IntLiteral)
    TOKEN_CONSTRUCTOR_IMP(f64, floating, Tkn_FloatLiteral)
    TOKEN_CONSTRUCTOR_IMP(char, character, Tkn_CharLiteral)
    TOKEN_CONSTRUCTOR_IMP(const std::string&, str, Tkn_StringLiteral)
    TOKEN_CONSTRUCTOR_IMP(ast::Ident*, ident, Tkn_Identifier);


    Token::~Token() {}

    const std::string& Token::get_string(TokenKind kind) {
        return token_strings[kind];
    }

    TokenKind Token::keyword(const std::string& str) {
        auto iter = std::find(token_strings.begin(), token_strings.end(), str);
        if(iter == token_strings.end())
            return Tkn_None;
        else {
            u64 index = std::distance(token_strings.begin(), iter);
            return (TokenKind) index;
        }
    }

	std::ostream& operator<< (std::ostream& out, const Token& t) {
		out << "Token( " << token_strings[t.tokenKind] << " ";
	    
		switch (t.tokenKind) {
            case Tkn_IntLiteral:
                out << "(" << t.integer << ")";
                break;
            case Tkn_FloatLiteral:
                out << "(" << t.floating << ")";
                break;
            case Tkn_StringLiteral:
                out << "(" << t.str << ")";
                break;
            case Tkn_CharLiteral:
                out << "(" << t.character << ")";
                break;
            case Tkn_Identifier:
                out << "(" << t.ident->value->value() << ")";
                break;
		}
		out << ", " 
            << t.position.line <<  ", "
            << t.position.column << ", "
            << t.position.span << ", "
            << t.position.fileId << ")";

		return out;
	}
}

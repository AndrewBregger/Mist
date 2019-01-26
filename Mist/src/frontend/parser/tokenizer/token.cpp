#include "token.hpp"
#include <vector>
#include <algorithm>
#include "interpreter.hpp"

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
    TOKEN_CONSTRUCTOR_IMP(ast::Ident*, ident, Tkn_Identifier)


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
            default:
                break;
		}
		out << ", "
            << t.position.line <<  ", "
            << t.position.column << ", "
            << t.position.span << ", "
            << t.position.fileId << ")";

		return out;
	}

    i32 Token::prec() {
        switch(kind()) {
            case Tkn_AstrickAstrick:
                return 12;
            case Tkn_Slash:
            case Tkn_Astrick:
            case Tkn_Percent:
                return 11;
            case Tkn_Plus:
            case Tkn_Minus:
                return 10;
            case Tkn_LessLess:
            case Tkn_GreaterGreater:
                return 9;
            case Tkn_EqualEqual:
            case Tkn_BangEqual:
                return 8;
            case Tkn_LessEqual:
            case Tkn_GreaterEqual:
            case Tkn_Less:
            case Tkn_Greater:
                return 7;
            case Tkn_Ampersand:
                return 6;
            case Tkn_Carrot:
                return 5;
            case Tkn_Pipe:
                return 4;
            case Tkn_And:
                return 3;
            case Tkn_Or:
                return 2;
        // case Tkn_Question:
        // case Tkn_PeriodPeriod:
        // case Tkn_PeriodPeriodPeriod:
        // case Tkn_InfixOp:
        //   return 2;
            case Tkn_Equal:
            case Tkn_LessLessEqual:
            case Tkn_GreaterGreaterEqual:
            case Tkn_PlusEqual:
            case Tkn_MinusEqual:
            case Tkn_SlashEqual:
            case Tkn_AstrickEqual:
            case Tkn_AmpersandEqual:
            case Tkn_PipeEqual:
            case Tkn_CarrotEqual:
            case Tkn_AstrickAstrickEqual:
            case Tkn_PeriodPeriod:
            case Tkn_Dollar:
                return 1;
            default:
                return 0;
        }
    }

    Associative Token::acc() {
        switch(kind()) {
            case Tkn_AstrickAstrick:
            case Tkn_Dollar:
                return Associative::Right;
            case Tkn_Slash:
            case Tkn_Astrick:
            case Tkn_Percent:
            case Tkn_Plus:
            case Tkn_Minus:
            case Tkn_LessLess:
            case Tkn_GreaterGreater:
            case Tkn_EqualEqual:
            case Tkn_BangEqual:
            case Tkn_LessEqual:
            case Tkn_GreaterEqual:
            case Tkn_Less:
            case Tkn_Greater:
            case Tkn_Ampersand:
            case Tkn_Carrot:
            case Tkn_Pipe:
            case Tkn_And:
            case Tkn_Or:
            case Tkn_Equal:
            case Tkn_LessLessEqual:
            case Tkn_GreaterGreaterEqual:
            case Tkn_PlusEqual:
            case Tkn_MinusEqual:
            case Tkn_SlashEqual:
            case Tkn_AstrickEqual:
            case Tkn_AmpersandEqual:
            case Tkn_PipeEqual:
            case Tkn_CarrotEqual:
            case Tkn_AstrickAstrickEqual:
            case Tkn_PeriodPeriod:
                return Associative::Left;
            default:
                return Associative::None;
        }
    }

    bool Token::is_operator() {
        auto k = kind();
        return Tkn_Plus <= k && k <= Tkn_PipeEqual;
    }

    bool Token::is_assignment() {
        auto k = kind();
        return Tkn_Equal <= k && k <= Tkn_PipeEqual;
    }
}

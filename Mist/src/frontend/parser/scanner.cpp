#include "scanner.hpp"

#include <thread>
#include <iostream>

namespace mist {
    Scanner::Scanner(Interpreter* interp) : interp(interp) {}

    Scanner::~Scanner() = default;

    void Scanner::init(io::File* file) {
        this->file = file;
        if (!init()) {
			// report error
		}
    }

    void Scanner::advance() {
        current = next_token();
    }

    Token& Scanner::token() {
		return current;
    }


    bool Scanner::init() {

		// this is unecessary complexity, but I want to try it.
		//std::thread loadThread([](io::File* file) {
		//}, this->file);

        index = 0;
        position = mist::Pos(0, 0, 0, file->id());

		if (!file->load()) return false;
	
            
        // std::cout << "Waiting for the file to load" << std::endl;
		// this will join the calling thread to wait until loadThread is complete.
		//loadThread.join();

        // std::cout << "The file has been loaded" << std::endl;

        source = &file->value();
        currentCh = &source->at(index);
        if(index + 1 >= source->size())
            nextCh = nullptr;
        else
            nextCh = &source->at(index + 1);
		return true;
    }

    void Scanner::bump() {
        if(index + 1 < source->size()) {
            // check if the current character is a new line
            if(check('\n')) {
                // if so, then update the line and column count
                position.line++;
                position.column = 0;
            }
    
            // move the current forward in the source
            ++index;
            ++position.span;
            ++savePos.span;
            ++position.column;
    
            // update the character pointers.
            currentCh = &source->at(index);
            if(index + 1 >= source->size())
                nextCh = nullptr;
            else
                nextCh = &source->at(index + 1);
        }
        else
            currentCh = nullptr;
    }


    Token Scanner::next_token() {
        // consumes all of the whitespace
        while(currentCh and isspace(*currentCh) and !check('\n'))
            bump();
       
        // initializes a new token from the currnet point in the text
        new_token();

		if (!currentCh)
			return Token(Tkn_Eof, savePos);

		if (isalpha(*this->currentCh) or check('_'))
			return scan_identifier();
		else if (isdigit(*this->currentCh))
			return scan_number_literal();
		else
			return scan_op();
    }

	Token Scanner::scan_identifier() {
        std::string temp;
        while(isalnum(*currentCh) or check('_')) {
            temp.push_back(*currentCh);
            bump();
        }

        // clean up the string if it isnt used by the table

        
        TokenKind kind = Token::keyword(temp);

        if(kind == Tkn_None) {
			auto s = interp->find_string(temp);
			// this is fine, Visual Studio is not detecting the constructors generated from a Macro.
			return Token(new ast::Ident(s, savePos), savePos);
        }
        else {
            return Token(kind, savePos);
        }
    }

    Token Scanner::scan_number_literal() {
        std::string temp;
		if (check('0')) {
            bump();

			if (check('x') or
				check('X')) {
                bump();
				while (isxdigit(*currentCh)){
                    temp.push_back(*currentCh);
                    bump();
                }

				u64 val = strtoll(temp.c_str(), NULL, 16);
				return Token(val, savePos);
			}
			else if (check('b') or
                     check('B')) {
                bump();
				while (check('1') or
					   check('0')) {
                    temp.push_back(*currentCh);
                    bump();
                }

				u64 val = strtoll(temp.c_str(), NULL, 2);
				return Token(val, savePos);
			}
            else
                temp.push_back('0');
		}
		bool floating_point = false;
		bool scientific_notation = false;
        // scan digits before the decimal point
		while (isdigit(*currentCh)) {
            temp.push_back(*currentCh);
            bump();
        }

		if (check('.') and (nextCh and *nextCh != '.')) {
            temp.push_back(*currentCh);
            bump();
			while (isdigit(*currentCh)) {
                temp.push_back(*currentCh);
                bump();
            }
			floating_point = true;
		}
		if (check('e') or
			check('E')) {
            temp.push_back(*currentCh);
            bump();
			if (check('-') or
				check('+')) {
                if(check('-'))
                    floating_point = true;
                temp.push_back(*currentCh);
                bump();
            }



			if (isdigit(*currentCh))
				while(isdigit(*currentCh)) {
                    temp.push_back(*currentCh);
                    bump();
                }
			else {
				// report errors
				//scan_error(scanner->file, scanner->line, scanner->column,
			// 		"Missing exponent in float literal\n");
			}
		}
    
		Token token;
		if (floating_point) {
			f64 val = strtod(temp.c_str(), NULL);
            token = Token(val, savePos);
		}
		else {
			u64 val = strtoll(temp.c_str(), NULL, 10);
            token = Token(val, savePos);
		}
		return token;
    }

#define SingleToken(ch, kind) case ch: return Token(kind, savePos);

#define DoubleToken(ch, kind1, kind2) \
	case ch: \
		if(check('=')) { \
            bump(); \
			return Token(kind2, savePos); \
		} \
		else { \
			return Token(kind1, savePos); \
		} \
		break;

#define TripleToken(ch, kind1, kind2, kind3) \
	case ch: \
		if(check('=')) { \
            bump(); \
			return Token(kind2, savePos); \
		} \
		else if(check((ch))) { \
            bump(); \
			return Token(kind3, savePos); \
		} \
		else { \
			return Token(kind1, savePos); \
		} \
		break;

#define FourToken(ch, kind1, kind2, kind3, kind4) \
	case ch: {\
		if(check('=')) { \
            bump(); \
			return Token(kind2, savePos); \
		} \
		else if(check((ch))) { \
            bump(); \
			if(check('=')) { \
                bump(); \
                return Token(kind4, savePos); \
			} \
			else { \
                return Token(kind3, savePos); \
			} \
		} \
		else { \
          return Token(kind1, savePos); \
		} \
	} break;

    Token Scanner::scan_op() {
        auto ch = *currentCh;
        bump();
        switch(ch) {
			case '\n': return Token(Tkn_NewLine, savePos);
            SingleToken('(', Tkn_OpenParen);
            SingleToken(')', Tkn_CloseParen);
            SingleToken('[', Tkn_OpenBrace);
            SingleToken(']', Tkn_CloseBrace);
            SingleToken('{', Tkn_OpenBracket);
            SingleToken('}', Tkn_CloseBracket);
            SingleToken(',', Tkn_Comma);
            SingleToken('~', Tkn_Tilde);
            SingleToken(';', Tkn_Semicolon);
            SingleToken('_', Tkn_Underscore);
            SingleToken('#', Tkn_Hash);

            DoubleToken('^', Tkn_Carrot, Tkn_CarrotEqual)
            DoubleToken('%', Tkn_Percent, Tkn_PercentEqual)
            DoubleToken('!', Tkn_Bang, Tkn_BangEqual)
            DoubleToken('=', Tkn_Equal, Tkn_EqualEqual)
            DoubleToken('+', Tkn_Plus, Tkn_PlusEqual)

            TripleToken('|', Tkn_Pipe, Tkn_PipeEqual, Tkn_Or)
            TripleToken('&', Tkn_Ampersand, Tkn_AmpersandEqual, Tkn_And)
            TripleToken(':', Tkn_Colon, Tkn_ColonEqual, Tkn_ColonColon);

			FourToken('*', Tkn_Astrick, Tkn_AstrickEqual, Tkn_AstrickAstrick, Tkn_AstrickAstrickEqual)
			FourToken('>', Tkn_Greater, Tkn_GreaterEqual, Tkn_GreaterGreater, Tkn_GreaterGreaterEqual)

            case '-': {
                    if(check('>')) {
                        bump();
                        return Token(Tkn_MinusGreater, savePos);
                    }
                    else
                        return Token(Tkn_Minus, savePos);
                }

            case '.': {
                    if(check('.')) {
                        bump();
                        return Token(Tkn_PeriodPeriod, savePos);
                    }
                    return Token(Tkn_Period, savePos);
                }
            case '/': {
                    if(check('/') or check('*'))
                        return scan_comment();
                    else if(check('='))
                        return Token(Tkn_SlashEqual, savePos);
                    else
                        return Token(Tkn_Slash, savePos);
                    }
                  case '\'': {
                            return scan_character();
                        }
                  case '"': {
                            return scan_string();
                    }
			// FourToken('<', Tkn_Less, Tkn_LessEqual, Tkn_LessLess, Tkn_LessLessEqual)
            case '<': {
                if(check('=')) {
                    bump();
                    return Token(Tkn_LessEqual, savePos);
                }
                else if(check('<')) {
                    bump();
                    if(check('=')) {
                        bump();
                        return Token(Tkn_LessLessEqual, savePos);
                    }
                    else {
                        return Token(Tkn_LessLess, savePos);
                    }
                }
                else if(check('>')) {
                    bump();
                    return Token(Tkn_Unit, savePos);
                }
                else {
                  return Token(Tkn_Less, savePos);
                }
            } break;
            default:
              std::cout << "Found unknown character: '" << ch << "'" << std::endl;
              break;  
        }

        return Token(Tkn_Error, savePos);
    }

    Token Scanner::scan_character() {
        char temp;
        switch(*currentCh) {
            case '\\':
                temp = validate_escape();
                break;
            default:
                temp = *currentCh;
                bump();
        }
		if(check('\''))
			bump();
		else {
			// report the error
			std::cout << "This is an error" << std::endl;
		}
		// this is fine, Visual Studio is not detecting the constructors generated from a Macro.
        return Token(temp, savePos);
    }

    Token Scanner::scan_string() {
		char ch;
        std::string temp;
        while(currentCh and !check('"')) {
            if(check('\\'))
                ch = validate_escape();
            else {
                ch = *currentCh;
                bump();
            }
    
            temp.push_back(ch);

			if (!currentCh)
				std::cout << "String current character is null" << std::endl;
        }
        bump();
		// this is fine, Visual Studio is not detecting the constructors generated from a Macro.
        return Token(temp, savePos);
    }
    
    /// @TODO: implement scanning for block comments and embedded block comments.
    Token Scanner::scan_comment() {
        // consume the next forward slash
		bump();
        std::string temp;
		while (!check('\n')) {
			temp.push_back(*currentCh);
			bump();
		}

        // takes the new line
        bump();
        Token token = Token(temp, savePos);
        token.tokenKind = Tkn_Comment;
        return token;
    }

    char Scanner::validate_escape() {
        bump();               // consumes the the the forward slash
        auto ch = *currentCh; // gets the character following the forward slash.
		bump();               // consumes it.
        switch(ch) {
            case 'a':
                return 0x07;
            case 'b':
                return 0x08;
            case 'f':
                return 0x0C;
            case 'n':
                return 0x0A;
            case 'r':
                return 0x0D;
            case 't':
                return 0x09;
            case 'v':
                return 0x0B;
            case '\\':
                return 0x5C;
            case '\'':
                return 0x27;
            case '"':
                return 0x22;
            case '?':
                return 0x3F;
          /*
            case 'x':
                bump();
                while(isxdigit(curr)) bump();
                str = cursor->substr(start);
                pos.span = str.size();
                return EscapeCharecterType::Hex;
            case 'e':
                str.push_back(0x1B);
                return EscapeCharecterType::Character;
            case 'U':
                bump();
                while(isxdigit(curr)) bump();
                str = cursor->substr(start);
                pos.span = str.size();
                return EscapeCharecterType::UnicodeLong;
            case 'u':
                bump();
                while(isxdigit(curr)) bump();
                character = cursor->substr(start);
                pos.span = character.size();
                return EscapeCharecterType::UnicodeShort;
        */
            default:;
       //         scan_error(scanner->file, scanner->line, scanner->column, "invalid escape character: %c\n", Current(scanner));
        }
        return 0x0;
    }
}

#pragma once

#include "interpreter.hpp"
#include "token.hpp"

namespace mist {
    class Scanner {
        public:
            Scanner(mist::Interpreter* interp);

            ~Scanner();
    
            /// tokenizes the give file.
            // std::vector<Token> tokenize(io::File* file);
            
            // initializes the scanner with the new file
            void init(io::File* file);
    
            // advances to the next token
            void advance();
    
            // returns the most resent token
            Token& token();

        private:
    
            /// reset the scanner state for a new file.
            bool init();
        
            /// move the cursor to the next character.
            void bump();
    
            // this is to allow the span to be updated as we move along.
            inline void new_token() { position.span = 0; savePos = position; }

			inline bool check(char ch) { return *currentCh == ch; }


		private:

            Token next_token();
    
            Token scan_identifier();
            Token scan_number_literal();
            Token scan_op();
            Token scan_character();
            Token scan_string();
            Token scan_comment();

            static TokenKind keyword(const std::string& str);

            char validate_escape();

            mist::Interpreter* interp{nullptr}; /// the active interpreter
            io::File* file{nullptr}; /// the current file being scanned
            Token current;

            // state data
            u64 index;               /// the index within the source
            const char* currentCh;         /// the current character
            const char* nextCh;            /// the next character
            const std::string* source;     /// the source
            Pos position;            /// the current position within the file
            Pos savePos;             /// the start of current token
    };
}

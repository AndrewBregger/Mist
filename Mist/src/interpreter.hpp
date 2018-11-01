#pragma once

#include "common.hpp"
#include "utils/file.hpp"
#include "frontend/parser/ast/ast_common.hpp"

#include <unordered_map>
#include <cstdarg>
#include <vector>
#include <iostream>

namespace mist {
    class Parser;

    struct String {
        std::string val; 

        inline const std::string& value() { return val; }
    };

	class Context {
		public:
            Context(const std::vector<std::string>& args);
            
    
            /// Returns the root file (the file given as a parameter).
            io::File* root();
    
            /// looks if the file is create if it isnt then creates it
            io::File* load_file(const std::string& filename);
    
        
            /// gets a loaded file by id
            io::File* get_file(u64 id);
    
            /// looks for an existing string
            /// if it doesnt find one it creates it and returns it.
            /// if it does then it just returns that one.
            String* find_or_create_string(const std::string& str);
            
		private:
            /// creates a file of the given filename
            io::File* create_file(const std::string& filename);

    
            std::unordered_map<std::string, String*> stringTable;
            std::unordered_map<u64, io::File*> files;
            // Settings
            std::vector<std::string> args;
	};

	class Interpreter {
		public:
			Interpreter(const std::vector<std::string>& args);
			~Interpreter();

            void compile_root();

            String* find_string(const std::string& str);

            Parser* get_parser();
            void close_parser(Parser* p);

            template <typename... Args>
            void report_error(const Pos& pos, const std::string& msg, Args... args) {
        		auto file = context.get_file(pos.fileId);
        		std::cout << file->name() << ":" << pos.line << ":" << pos.column << "\t";
#if _WIN32
        		vprintf_s(msg.c_str(), va);
#else
                printf(msg.c_str(), args...);
#endif
        		std::cout << std::endl;
            }
		private:
			Context context;
            std::vector<std::pair<Parser*, bool>> parsers;
            // std::vector<Parser*> parsers;
	};
}


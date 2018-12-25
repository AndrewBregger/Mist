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
			/// Interpreter constructor
			/// @param args list of command line arguements
			Interpreter(const std::vector<std::string>& args);

			~Interpreter();

			/// Compiles the root file given from the commandline.
            void compile_root();

            /// Looks up a string in a string table
            ///		creates a String if it doesnt exist
            /// @param str the string to lookup
            String* find_string(const std::string& str);

            /// Checks if there is an available parser to use
            Parser* get_parser();

            /// Closes the parser and makes it as available to be used
            /// 	by get_parser
            void close_parser(Parser* p);

            inline bool has_error() { return errors != 0; }

            template <typename... Args>
            void report_error(const Pos& pos, const std::string& msg, Args... args) {
        		auto file = context.get_file(pos.fileId);
        		std::cout << file->name() << ":" << pos.line << ":" << pos.column << "\t";
        		report_error(msg, args...);
            }

            template <typename... Args>
            void report_error(const std::string& msg, Args... args) {
#if _WIN32
                printf_s(msg.c_str(), args...);
#else
                printf(msg.c_str(), args...);
#endif
                std::cout << std::endl;
                errors++;
            }
		private:
			Context context;
            std::vector<std::pair<Parser*, bool>> parsers;
            // std::vector<Parser*> parsers;
            // Metrics
            u32 errors{0};
	};
}


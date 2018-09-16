#pragma once

#include "common.hpp"
#include <vector>

#include "utils/file.hpp"

#include <unordered_map>


namespace mist {

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

		private:
			Context context;
	};
}


#include "interpreter.hpp"
#include <iostream>

#include <windows.h>

const u32 BUFFER_SIZE = 128; // if it larger then 128 for now then there is problem

namespace mist {
    // @TODO: parser the command line arguments
    Context::Context(const std::vector<std::string>& args) : args(args) {
    }
    
    
    io::File* Context::root() {
        // for now it is assumed the file is the first argument     
        auto& filename = args.front();

		return load_file(filename);
    }

    io::File* Context::load_file(const std::string& filename) {
        char tmpBuffer[BUFFER_SIZE];
		char** ttBuffer = nullptr;

        auto res = GetFullPathNameA(
                filename.c_str(),
                BUFFER_SIZE, // find a way to get the length the path.
                tmpBuffer, 
                ttBuffer
            );
        
        if(!res)
            return nullptr;

        
        // absolute path of the given file
        auto name = std::string(tmpBuffer);
        
        u64 hash = io::File::hash_filename(name);
        auto iter = files.find(hash);
        if(iter == files.end())
            return create_file(name);
        return iter->second;
    }

    io::File* Context::get_file(u64 id) {
        auto iter = files.find(id);
        if(iter == files.end())
            return nullptr;
        return iter->second;
    }


    io::File* Context::create_file(const std::string& filename) {
        io::File* file = new io::File(filename);
        files.emplace(file->id(), file);
        return file;
    }

	String* Context::find_or_create_string(const std::string& str) {
		auto iter = stringTable.find(str);
		if (iter != stringTable.end())
			return iter->second;

		/// creating this struct with an allocator
		mist::String* s = new mist::String;
		s->val = str;
		stringTable.emplace(str, s);
		return s;
	}

    Interpreter::Interpreter(const std::vector<std::string>& args) : context(args) {
    }

    Interpreter::~Interpreter() {
    }

    void Interpreter::compile_root() {
        auto root = context.root();
    }


    String* Interpreter::find_string(const std::string& str) {
        return context.find_or_create_string(str);
    }
}

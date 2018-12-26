#include "interpreter.hpp" 

#include "frontend/parser/tokenizer/scanner.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/parser/ast/ast_printer.hpp"

#ifdef _WIN32
    #include <windows.h>
#endif

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
        int res = 0;

#ifdef _WIN32
        res = GetFullPathNameA(
                filename.c_str(),
                BUFFER_SIZE, // find a way to get the length the path.
                tmpBuffer, 
                ttBuffer
            );
#else
        char* t = realpath(filename.c_str(), tmpBuffer);
        if(t)
            res = 1;
#endif
        
        if(!res)
            return create_file(filename);

        
        // absolute path of the given file
        auto name = std::string(tmpBuffer);
        
        u64 hash = io::File::hash_filename(name);
        auto file = get_file(hash);
        if(file)
            return file;
        else
            return create_file(name);
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

        auto p = get_parser();

        auto m = p->parse_root(root);

//        std::cout << "Parsing Expression" << std::endl;
        auto e = p->parse_test();

        ast::print(std::cout, e);

        close_parser(p);
    }

    Parser* Interpreter::get_parser() {
        for(auto& x : parsers) {
            if(!x.second) {
                x.second = true;
                return x.first;
            }
        }
        auto p = new mist::Parser(this);
        parsers.emplace_back(p, true);
        return p; 
    }

    void Interpreter::close_parser(Parser* p) {
        for(auto& x : parsers)
            if(x.first == p)
                x.second = false;
    }


    String* Interpreter::find_string(const std::string& str) {
        return context.find_or_create_string(str);
    }

// //#pragma optimize("", off)
//     void Interpreter::report_error(const mist::Pos& pos, const std::string& msg, ...) {
// 		va_list va;
// 		const char* m = msg.c_str();
// 		va_start(va, m);
// 		report_error(pos, msg, va);
// 		va_end(va);
//     }
// //#pragma optimize("", on)

//     void Interpreter::report_error(const mist::Pos& pos, const std::string& msg, va_list va) {
// 		auto file = context.get_file(pos.fileId);

// 		std::cout << file->name() << ":" << pos.line << ":" << pos.column << "\t";

// #if _WIN32
// 		vprintf_s(msg.c_str(), va);
// #else
//         vprintf(msg.c_str(), va);
// #endif

// 		std::cout << std::endl;
// 		// print file line and location information
//     }
}

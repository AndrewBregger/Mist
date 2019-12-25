#include "interpreter.hpp"
#include "frontend/checker/types/type.hpp"
#include "frontend/checker/typer.hpp"

#include "frontend/parser/tokenizer/scanner.hpp"
#include "frontend/parser/parser.hpp"
#include "frontend/parser/ast/ast_printer.hpp"

#include "backend/bytecodegenerator.hpp"


#ifdef _WIN32
    #include <windows.h>
#endif

mist::Type* type_u8;
mist::Type* type_u16;
mist::Type* type_u32;
mist::Type* type_u64;
mist::Type* type_i8;
mist::Type* type_i16;
mist::Type* type_i32;
mist::Type* type_i64;
mist::Type* type_f32;
mist::Type* type_f64;
mist::Type* type_char;
mist::Type* type_bool;
mist::Type* type_unit;
mist::Type* type_emptytuple;

const u32 BUFFER_SIZE = 128; // if it larger then 128 for now then there is problem

namespace mist {
    // @TODO: parser the command line arguments
    Context::Context(const std::vector<std::string>& args) : args(args), directory(fs::current_path(), true) {
        directory.load();
        for(auto x : args)
            std::cout << x << std::endl;
    }
    
    
    io::File* Context::root() {
        // for now it is assumed the file is the first argument     
        auto& filename = args.front();

		return load_file(filename);
    }

    io::File* Context::load_file(const std::string& filename) {
        fs::path p(filename);
        u64 hash = io::FileIO::hash_name(p);

        return get_file(hash);
    }

    io::File* Context::get_file(u64 id) {
        auto [file, valid] = directory.find(id);
        if(valid) {
            if(file->is_file()) {
                file->load();
                return static_cast<io::File*>(file);
            }
            std::clog << "[LOG]: Attempted to load a directory" << std::endl;
        }
        return nullptr;
    }

	struct String* Context::find_or_create_string(const std::string& str) {
		auto iter = stringTable.find(str);
		if (iter != stringTable.end())
			return iter->second;

		/// creating this struct with an allocator
		auto s = new struct mist::String;
		s->val = str;
		stringTable.emplace(str, s);
		return s;
	}

    bool Context::has_root() {
        return !args.empty();
    }

    Interpreter::Interpreter(const std::vector<std::string>& args) : context(args), typer(new Typer(this)) {
        init_strings();
        init_type_table();
    }

    Interpreter::~Interpreter() {
    }

    void Interpreter::compile_root() {
        auto root = context.root();

        if(root->is_load())
            root->load();

        if(root->value().size() == 0) {
            report_error("given file is empty, not compiling");
            return;
        }



        auto p = get_parser();

        typer->create_root();

        auto m = p->parse_root(root);

        close_parser(p);

        if(has_error()) {
            std::cout << "There was an error: Exiting" << std::endl;
            close_parser(p);
            return;
        }

//        auto e = p->parse_expr();

        typer->resolve_module(m);

        if(has_error()) {
            return;
        }
//
//        BytecodeGenerator codegen(this);
//
//        codegen.run(m);
    }

    io::File *Interpreter::find_file(const std::string &name) {
        // maybe this should search for a file by not assuming name is the entire path.
        return context.load_file(name);
    }

    Parser* Interpreter::get_parser() {
        for(auto& x : parsers) {
            if(!x.second) {
                x.second = true;
                return x.first;
            }
        }
        auto p = new mist::Parser(typer, this);
        parsers.emplace_back(p, true);
        return p; 
    }

    void Interpreter::close_parser(Parser* p) {
        for(auto& x : parsers)
            if(x.first == p)
                x.second = false;
    }

    void Interpreter::init_strings() {
        context.find_or_create_string("u8");
        context.find_or_create_string("u16");
        context.find_or_create_string("u32");
        context.find_or_create_string("u64");

        context.find_or_create_string("i8");
        context.find_or_create_string("i16");
        context.find_or_create_string("i32");
        context.find_or_create_string("i64");


        context.find_or_create_string("f32");
        context.find_or_create_string("f64");


        context.find_or_create_string("char");
        context.find_or_create_string("bool");
        context.find_or_create_string("Unit");
    }

    // I dont this doesnt make since and is complicated.
    void Interpreter::init_type_table() {
        type_u8   =  add_type(new PrimitiveType(PrimitiveKind::U8,  context.find_or_create_string("u8"), 1));
        type_u16  =  add_type(new PrimitiveType(PrimitiveKind::U16, context.find_or_create_string("u16"), 2));
        type_u32  =  add_type(new PrimitiveType(PrimitiveKind::U32, context.find_or_create_string("u32"), 4));
        type_u64  =  add_type(new PrimitiveType(PrimitiveKind::U64, context.find_or_create_string("u64"), 8));
        type_i8   =  add_type(new PrimitiveType(PrimitiveKind::I8,  context.find_or_create_string("i8"), 1));
        type_i16  =  add_type(new PrimitiveType(PrimitiveKind::I16, context.find_or_create_string("i16"), 2));
        type_i32  =  add_type(new PrimitiveType(PrimitiveKind::I32, context.find_or_create_string("i32"), 4));
        type_i64  =  add_type(new PrimitiveType(PrimitiveKind::I64, context.find_or_create_string("i64"), 8));
        type_f32  =  add_type(new PrimitiveType(PrimitiveKind::F32, context.find_or_create_string("f32"), 4));
        type_f64  =  add_type(new PrimitiveType(PrimitiveKind::F64, context.find_or_create_string("f64"), 8));
        type_char =  add_type(new PrimitiveType(PrimitiveKind::Char, context.find_or_create_string("char"), 1));
        type_bool =  add_type(new PrimitiveType(PrimitiveKind::Bool, context.find_or_create_string("bool"), 1));
        type_emptytuple = add_type(new TupleType(std::vector<Type*>(), 0));
        type_unit = add_type(new UnitType());

        typer->add_prelude(static_cast<PrimitiveType*>(type_u8)->name(),    type_u8, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_u16)->name(),   type_u16, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_u32)->name(),   type_u32, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_u64)->name(),   type_u64, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_i8)->name(),    type_i8, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_i16)->name(),   type_i16, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_i32)->name(),   type_i32, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_i64)->name(),   type_i64, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_f32)->name(),   type_f32, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_f64)->name(),   type_f64, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_char)->name(),  type_char, Value, Ty);
        typer->add_prelude(static_cast<PrimitiveType*>(type_bool)->name(),  type_bool, Value, Ty);
        typer->add_prelude(context.find_or_create_string("Unit"), type_unit, Value, Ty);
    }


    struct String* Interpreter::find_string(const std::string& str) {
        return context.find_or_create_string(str);
    }

    Type* Interpreter::add_type(Type *type) {
        types.insert(type);
        return type;
    }

    Type *Interpreter::find_type(Type *type) {
        if(!type)
            return nullptr;
        for(auto t : types) {
            if(typer->equivalent_type(t, type))
                return t;
        }
        return nullptr;
    }

    bool Interpreter::contains_equivalent_type(Type *type) {
        return find_type(type) != nullptr;
    }

    void Interpreter::print_line(Pos pos) {
        auto file = context.get_file(pos.fileId);

        auto line = file->get_line(pos.line);

        std::cout << ">\t" << line << std::endl;


        std::string l(pos.column - 1, '-');
        std::cout << ">\t" << l << "^" << std::endl;
    }

    void strip(std::string& temp) {
        int index = 0;
        for(auto c : temp)
            if(std::isspace(c))
                index++;

        for(auto end = temp.rbegin(); end != temp.rend(); ++end)
            if(std::isspace(*end))
                temp.pop_back();

        temp = temp.substr(index);
    }

    void Interpreter::start_repl() {
        std::string input;

        while(true) {
            std::cout << "> ";
            std::cin >> input;
            strip(input);

            std::cout << "Input: '" << input << "'" << std::endl;

            if(input.empty())
                continue;

            if(input.front() == ':') {
                switch(input[1]) {
                    case 'q':
                    case 'Q':
                        return;
                    default:
                        report_error("Invalid command: '%c'", input[1]);
                        break;
                }
            }

            // treat as code.
            auto parser = get_parser();


        }
    }

    void Interpreter::run_context() {
        if(context.has_root())
            compile_root();
        else
            start_repl();
    }
}


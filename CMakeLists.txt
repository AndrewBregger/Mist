cmake_minimum_required(VERSION 3.0)
project(mistc)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIR "Mist/src")

include_directories(${INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -pedantic")

set(SOURCE  ./Mist/src/interpreter.cpp
            ./Mist/src/utils/file.cpp
            ./Mist/src/frontend/parser/ast/ast.cpp
            ./Mist/src/frontend/parser/ast/ast_common.cpp
            ./Mist/src/frontend/parser/ast/ast_typespec.cpp
            ./Mist/src/frontend/parser/ast/ast_expr.cpp
            ./Mist/src/frontend/parser/ast/ast_decl.cpp
            ./Mist/src/frontend/parser/ast/ast_printer.cpp
            ./Mist/src/frontend/parser/tokenizer/scanner.cpp
            ./Mist/src/frontend/parser/tokenizer/token.cpp
            ./Mist/src/frontend/parser/parser.cpp
            ./Mist/src/main.cpp Mist/src/frontend/parser/parser.cpp Mist/src/frontend/parser/parser.hpp Mist/src/frontend/parser/ast/ast_pattern.cpp Mist/src/frontend/parser/ast/ast_pattern.hpp)

add_executable(mistc ${SOURCE})

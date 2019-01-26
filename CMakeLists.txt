cmake_minimum_required(VERSION 3.0)
project(mistc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INCLUDE_DIR "Mist/src")

include_directories(${INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wextra -DDEBUG")

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
            ./Mist/src/main.cpp Mist/src/frontend/parser/parser.cpp Mist/src/frontend/parser/parser.hpp Mist/src/frontend/parser/ast/ast_pattern.cpp Mist/src/frontend/parser/ast/ast_pattern.hpp Mist/src/frontend/checker/typer.cpp Mist/src/frontend/checker/typer.hpp Mist/src/frontend/checker/scope.cpp Mist/src/frontend/checker/scope.hpp Mist/src/frontend/checker/types/type.cpp Mist/src/frontend/checker/types/type.hpp Mist/src/utils/directory.cpp Mist/src/utils/directory.hpp Mist/src/utils/fileio.cpp Mist/src/utils/fileio.hpp)

add_executable(mistc ${SOURCE})
target_link_libraries(mistc stdc++fs)

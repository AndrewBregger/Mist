cmake_minimum_required(VERSION 3.0)
project(vm-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(VM_SOURCE   vm.hpp
                vm.cpp
                src/instruction.hpp
                src/opcode.hpp
                src/chunk.hpp
                src/value.hpp
                src/object.hpp
                src/instruction.cpp
                src/chunk.cpp
                src/value.cpp
                src/opcode.cpp
                src/object.cpp)

add_library(vmlib STATIC ${VM_SOURCE})

set_target_properties(vmlib PROPERTIES
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

link_directories("${PROJECT_SOURCE_DIR}/lib")

add_executable(vm-test main.cpp)
set_target_properties(vm-test PROPERTIES
    CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

TARGET_LINK_LIBRARIES(vm-test vmlib)

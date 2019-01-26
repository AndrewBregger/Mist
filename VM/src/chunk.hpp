#pragma once

#define CHUNK_SIZE 1024

#include <vector>

#include "opcode.hpp"
#include "value.hpp"
#include "instruction.hpp"

struct Chunk {
    u32 index;
    u32 count;

    std::vector<u8> code;
    std::vector<u32> line;
    std::vector<Value> constantTable;

    Chunk();

    Instruction load(u32 ip);

    void write_byte(u8 byte, u32 line);

    void write_instruction(const Instruction& inst, u32 l);

    u32 add_constant(const Value& value);
};

#pragma once

#include "common.hpp"
#include "opcode.hpp"

#include <optional>

struct Instruction {
    OpCode op;
    std::optional<u32> arg1;
    std::optional<u32> arg2;
    u32 sz;

    Instruction(OpCode op);
    Instruction(OpCode op, u32 arg1);
    Instruction(OpCode op, u32 arg1, u32 arg2);

    inline u32 size() { return sz;}

    u16 as_short();
};

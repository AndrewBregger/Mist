#include "chunk.hpp"
#include <iostream>


Chunk::Chunk() {
    index = 0;
    count = 0;
}

Instruction Chunk::load(u32 ip) {
    if(ip >= code.size()) {
        return Instruction(Error);
    }
    OpCode op = (OpCode) code[ip];
    ArgType arg = get_argtype(op);
    u8 arg1, arg2;
    switch(arg) {
        case A:
            arg1 = code[ip + 1];
            return Instruction(op, arg1);
        case AB:
            arg1 = code[ip + 1];
            arg2 = code[ip + 2];
            return Instruction(op, arg1, arg2);
        case None:
            return Instruction(op);
    }
    std::cout << "Invalid OpCode: " << op << std::endl;
    return Instruction(Error);
}

void Chunk::write_byte(uint8_t byte, u32 l) {
    code.push_back(byte);
    line.push_back(l);
}

void Chunk::write_instruction(const Instruction& inst, u32 l) {
    write_byte(inst.op, l);
    if(inst.arg1.has_value()) {
        write_byte(inst.arg1.value(), l);
    }
    if(inst.arg2.has_value()) {
        write_byte(inst.arg2.value(), l);
    }
}

u32 Chunk::add_constant(const Value& val) {
    constantTable.push_back(val);
    return constantTable.size() - 1;
}

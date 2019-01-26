#include "instruction.hpp"

Instruction::Instruction(OpCode op) : op(op), arg1(), arg2(), sz(1) {

}

Instruction::Instruction(OpCode op, u32 arg1) : op(op),
    arg1(std::move(arg1)), arg2(), sz(2) {

}

Instruction::Instruction(OpCode op, u32 arg1, u32 arg2) : op(op),
    arg1(std::move(arg1)), arg2(std::move(arg2)), sz(3) {

}

u16 Instruction::as_short() {
    // if there neither have a value then return -1;
    if(!arg1.has_value() && !arg2.has_value())
        return static_cast<u16>(-1);

    u16 harg1 = 0;
    u16 larg2 = 0;

    if(arg1.has_value())
         harg1 = arg1.value();
    if(arg2.has_value())
        larg2 = arg2.value();

    return (harg1 << 8) | (larg2 << 0);
}

#pragma once

#include "common.hpp"

enum ArgType {
    A,
    AB,
    None
};

enum OpCode : u8 {
    Load_Const,
    Load_Const_Int,
    Load_Const_Float,
    Load_True,
    Load_False,
    Load_Nill,

    Create_Local,
    Load_Local,
    Set_Local,

    Create_Global,
    Create_Glabal_Initialize,

    Load_Global,
    Set_Global,

    Get_Field,
    Set_Field,

    // this is to handle multiple returns from a Function
    // it could be used to handle var args as well.
    Create_Sequence,
    Unpack_Sequence,

    Create_Struct,
    Destroy_Struct,

    Cast_Double,
    Cast_Int,

    Call,

    Add_Int,
    Add_Double,

    Add_String,

    Minus_Int,
    Minus_Double,

    Multi_Int,
    Multi_Double,

    Div_Int,
    Div_Double,

    Mod_Int,
    Mod_Double,

    Pow_Int,
    Pow_Double,

    Less,
    LessEqual,

    Greater,
    GreaterEqual,

    Equal,

    Not,

    Pop,
    GoTo,
    JumpIfTrue,
    JumpIfFalse,
    Return,
    Quit,
    Error
};

ArgType get_argtype(OpCode op);

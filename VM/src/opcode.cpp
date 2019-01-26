#include "opcode.hpp"

ArgType get_argtype(OpCode op) {
    switch(op) {
        case Load_Const_Int:
        case Load_Const_Float:
        case Create_Global:
        case Return:
        case Create_Sequence:
        case Create_Struct:
            return A;
        case Call:
        case Load_Const:
        case GoTo:
        case Load_Global:
        case Set_Global:
        case Get_Field:
        case Set_Field:
        case Load_Local:
        case Set_Local:
            return AB;
        default:
            return None;
        // case Load_True:
        // case Load_False:
        // case Load_Nill:
        // case Add_Int:
        // case Add_Double:
        // case Add_String:
        // case Minus_Int:
        // case Minus_Double:
        // case Multi_Int:
        // case Multi_Double:
        // case Div_Int:
        // case Div_Double:
        // case Mod_Int:
        // case Mod_Double:
        // case Pow_Int:
        // case Pow_Double:
        // case Less:
        // case LessEqual:
        // case Greater:
        // case GreaterEqual:
        // case Equal:
        // case Not:
        // case JumpIfTrue:
        // case JumpIfFalse:
        // case Create_Local:
    }
}

#include "vm.hpp"
#include <cmath>
#include <iostream>
#include <cstdlib>

const u32 START_STACK = 1024;

#define NUMBER_CHECK(type, arg1, arg2, op) \
    if(!arg1.is_##type() && !arg2.is_##type()) { \
        std::cout << "Error: attempting '" << #op << "' with incompatable types" << std::endl; \
        return Execution_Error; \
    }

void vm_print_version() {
    std::cout << VM_MAJOR_VERSION << "." << VM_MINOR_VERSION << std::endl;
}

CallFrame::CallFrame(ObjLambda* fn, Value* stack, u32 ip) : fn(fn), ip(ip), fp(stack), locals(fn->arity + fn->num_locals) {
}

Instruction CallFrame::next() {
    return fn->chunk.load(ip);
}

Execution::Execution() {
    size = 0;
    cap = START_STACK;
    stack = (Value*) std::malloc(sizeof(u8) * cap);
    topStack = stack;
}

ExecutionStatus Execution::run(ObjLambda* lambda) {
    auto frame = new CallFrame(lambda, stack);
    frames.push(frame);
    return run();
}

u32 Execution::add_global(const Value& val) {
    globals.push_back(val);
    return globals.size() - 1;
}

Value* Execution::grow_stack() {
    if(size >= cap) {
        stack = (Value*) realloc(stack, cap * 2);
        cap *= 2;
    }
    return stack;
}

Value Execution::pop() {
    auto value = *(--topStack);
    size--;
    return value;
}

void Execution::push(const Value& val) {
    grow_stack();
    size++;
    *topStack++ = val;
}

Value& Execution::peek(int dist) {
    return topStack[-1 - dist];
}

 ExecutionStatus Execution::run() {
    if(frames.empty()) {
        std::cout << "Error: No call frame to execute" << std::endl;
    }

    activeFrame = frames.top();
    chunk = &activeFrame->fn->chunk;
    while(1) {
        // grab the next instruction
        Instruction inst = activeFrame->next();
        // increment the instruction pointer (index)
        activeFrame->ip += inst.size();

        switch(inst.op) {
            case Load_Const: {
                auto index = inst.as_short();
                if(index == -1) {
                    std::cout << "Error: Invalid Constant Index" << std::endl;
                    return Execution_Error;
                }
                push(chunk->constantTable[index]);
            } break;
            // case Load_Const_Int:
            //     break;
            // case Load_Const_Float:
            //     break;
            case Load_True:
                push(Value(true));
                break;
            case Load_False:
                push(Value(false));
                break;
            case Load_Nill:
                push(Value());
               break;
            case Create_Struct: {
                auto num = inst.arg1.value();
                std::vector<Value> members;
                members.reserve(num);
                while(num--)
                    members.push_back(pop());
                auto s = new ObjStruct(members.data(), members.size());
                push(Value(s));
            } break;
            case Destroy_Struct: {
                if(peek().is_object()) {
                    if(peek().obj->kind() == StructObj) {
                        Value val = pop();
                        delete val.obj;
                    }
                }
                std::cout << "Error: Unable to destroy a non struct object value" << std::endl;
                return Execution_Error;
            } break;
            case Cast_Double: {
                // if already double do nothing
                if(peek().is_float())
                    break;
                Value val = pop();
                if(val.is_integer()) {
                    push(Value((f64)val.i));
                }
                else if(val.is_boolean()) {
                    push(Value((f64)val.b));
                }
                else {
                    std::cout << "Error: Unable to cast an object";
                    return Execution_Error;
                }
            } break;
            case Cast_Int: {
                if(peek().is_integer())
                    break;
                Value val = pop();
                if(val.is_float()) {
                    push(Value((i64)val.i));
                }
                else if(val.is_boolean()) {
                    push(Value((i64)val.b));
                }
                else {
                    std::cout << "Error: Unable to cast an object";
                    return Execution_Error;
                }
            } break;
            case Create_Sequence: {
                u8 num = inst.arg1.value();
                std::vector<Value> elems;
                // elems.reserve(num);
                while(num--)
                    elems.push_back(pop());
                push(Value(elems.data(), elems.size()));
            } break;
            case Unpack_Sequence: {
                Value top = pop();
                if(top.is_sequence()) {
                    for(u32 i = top.seq.num - 1; i >= 0; --i)
                        push(top.seq.elems[i]);
                }
                else {
                    std::cout << "Unable to unpack a non sequence value" << std::endl;
                    return Execution_Error;
                }
            } break;
            // this must be called before a local is used.
            // it uses the top of the stack to intialize the value.
            case Create_Local:
                activeFrame->locals.push_back(pop());
                break;
            case Load_Local: {
                auto index = inst.as_short();
                if(index < activeFrame->locals.size())
                    push(activeFrame->locals[index]);
                else {
                    std::cout << "Error: accessing invalid local" << std::endl;
                    return Execution_Error;
                }
            } break;
            case Set_Local: {
                Value top = pop();
                auto index = inst.arg1.value();
                if(index < activeFrame->locals.size())
                    activeFrame->locals[index] = top;
                else {
                    std::cout << "Error: assigning to invalid local" << std::endl;
                    return Execution_Error;
                }
            } break;
            case Create_Global:
                add_global(Value());
                break;
            case Create_Glabal_Initialize: {
                Value val = pop();
                add_global(val);
            } break;
            case Load_Global: {
                auto index = inst.as_short();
                if(index < globals.size())
                    push(globals[index]);
                else {
                    std::cout << "Invalid global index" << std::endl;
                    return Execution_Error;
                }
            } break;
            case Set_Global: {
                auto value = pop();
                auto index = inst.as_short();
                if(index < globals.size())
                    globals[index] = value;
                else {
                    std::cout << "Unable to set global, invalid global index" << std::endl;
                    return Execution_Error;
                }
            } break;
            case Get_Field: {
                Value val = peek();
                if(val.is_object()) {
                    if(val.obj->kind() == StructObj) {
                        auto s = (ObjStruct*) val.obj;
                        auto index = inst.as_short();
                        Value* field = s->attr(index);
                        if(field)
                            push(field);
                        else {
                            std::cout << "Error: Invalid field index on a struct object" << std::endl;
                            return Execution_Error;
                        }
                    }
                }
                std::cout << "Error: Unable to access the field of non struct object" << std::endl;
                return Execution_Error;
            } break;
            case Set_Field: {

            } break;
            case Call: {
                auto params = inst.as_short();

                std::vector<Value> paramValues;
                paramValues.reserve(params);

                while(params--)
                    paramValues.push_back(pop());

                // the top of the stack should be the calling
                // object. (function)
                Value fn = pop();
                if(fn.is_object()) {
                    std::cout << fn << std::endl;
                    Object* obj = fn.obj;
                    if(obj->kind() == LambdaObj) {
                        auto fn = (ObjLambda*) obj;
                        if(fn->arity != paramValues.size()) {
                            std::cout << "Error: Invalid number of parameters for function: "
                                "expecting: '" << fn->arity << "' found: '" << paramValues.size() << "'" << std::endl;
                            return Execution_Error;
                        }
                        call(fn, paramValues);
                        break;
                    }
                }
                std::cout << "Attempting to call a non lambda value" << std::endl;
                std::cout << "TOP: " << fn << std::endl;
                return Execution_Error;
            } break;
            case Add_Int: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(integer, arg1, arg2, +)
                push(Value(arg1.i + arg2.i));
            } break;
            case Add_Double: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(float, arg1, arg2, +)
                push(Value(arg1.f + arg2.f));
            } break;
            case Add_String:
                break;
            case Minus_Int: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(integer, arg1, arg2, -)
                push(Value(arg1.i - arg2.i));
            } break;
            case Minus_Double: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(float, arg1, arg2, -)
                push(Value(arg1.f - arg2.f));
            } break;
            case Multi_Int: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(integer, arg1, arg2, *)
                push(Value(arg1.i * arg2.i));
            } break;
            case Multi_Double: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(float, arg1, arg2, *)
                push(Value(arg1.f * arg2.f));
            } break;
            case Div_Int: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(integer, arg1, arg2, /)
                push(Value(arg1.i / arg2.i));
            } break;
            case Div_Double: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(float, arg1, arg2, /)
                push(Value(arg1.f / arg2.f));
            } break;
            case Mod_Int: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(integer, arg1, arg2, %)
                push(Value(arg1.i % arg2.i));
            } break;
            case Mod_Double: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(float, arg1, arg2, %)
                push(Value(fmodf(arg1.f, arg2.f)));
            } break;
            case Pow_Int: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(integer, arg1, arg2, pow)
                push(Value(pow(arg1.i, arg2.i)));
            } break;
            case Pow_Double: {
                Value arg1 = pop();
                Value arg2 = pop();
                NUMBER_CHECK(float, arg1, arg2, pow)
                push(Value(powf(arg1.f, arg2.f)));
            } break;
            case Less: {
                Value arg1 = pop();
                Value arg2 = pop();

                if(arg1.is_integer()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.i < arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.i < arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_float()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.f < arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.f < arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_object()) {
                    // @TODO: Implemented later
                }
            }
                break;
            case LessEqual: {
                Value arg1 = pop();
                Value arg2 = pop();

                if(arg1.is_integer()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.i <= arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.i <= arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_float()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.f <= arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.f <= arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_object()) {
                    // @TODO: Implemented later
                }
            }
                break;
            case Greater: {
                Value arg1 = pop();
                Value arg2 = pop();

                if(arg1.is_integer()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.i > arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.i > arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_float()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.f > arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.f > arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_object()) {
                    // @TODO: Implemented later
                }
            }
                break;
            case GreaterEqual: {
                Value arg1 = pop();
                Value arg2 = pop();

                if(arg1.is_integer()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.i >= arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.i >= arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_float()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.f >= arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.f >= arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_object()) {
                    // @TODO: Implemented later
                }
            }
                break;
            case Equal: {
                Value arg1 = pop();
                Value arg2 = pop();

                if(arg1.is_integer()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.i == arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.i == arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_float()) {
                    if(arg2.is_integer()) {
                        push(Value(arg1.f == arg2.i));
                    }
                    else if(arg2.is_float()) {
                        push(Value(arg1.f == arg2.f));
                    }
                    else {
                        std::cout << "Error: comparing integer an non integral type" << std::endl;
                        return Execution_Error;
                        // what should happen here
                    }
                }
                else if(arg1.is_object()) {
                    // @TODO: Implemented later
                }
            } break;
            case Not: {
                Value arg = pop();
                if(arg.is_boolean()) {
                    push(Value(!arg.b));
                }
                else {
                    std::cout << "Invalid argument to not" << std::endl;
                    return Execution_Error;
                }
            } break;
            case Return:
                // clean up current frame
                delete activeFrame;
                frames.pop();

                if(frames.empty()) {
                    activeFrame = nullptr;
                    chunk = nullptr;
                    return Execution_Complete;
                }

                activeFrame = frames.top();
                chunk = &activeFrame->fn->chunk;
                // maybe reset the stack pointer to where the initial call started
                break;
            case GoTo: {
                i16 nip = (i16) inst.as_short();
                activeFrame->ip += nip;
            } break;
            // refactor these to to handle other types of values.
            case JumpIfTrue: {
                Value& val = peek();
                if(val.is_boolean()) {
                    if(val.b) {
                        activeFrame->ip += (i16) inst.as_short();
                    }
                }
                else {
                    std::cout << "Expecting a boolean at top of stack" << std::endl;
                    return Execution_Error;
                }
            } break;
            case JumpIfFalse: {
                Value& val = peek();
                if(val.is_boolean()) {
                    if(val.b == false) {
                        activeFrame->ip += (i16) inst.as_short();
                    }
                }
                else {
                    std::cout << "Expecting a boolean at top of stack" << std::endl;
                    return Execution_Error;
                }
            } break;
            case Pop:
                pop();
                break;
            case Error:
                break;
            case Quit:
                return Execution_Complete;
        }
    }
}

void Execution::call(ObjLambda* lambda, const std::vector<Value>& params) {
    auto frame = new CallFrame(lambda, stack);
    activeFrame = frame;
    chunk = &activeFrame->fn->chunk;
    frames.push(frame);

    // set the values of the input parameters
    for(i32 i = lambda->arity - 1, index = 0; i >= 0;  --i, ++index) {
        frame->locals[index] = params[i];
    }
}

#pragma once

#define VM_MAJOR_VERSION 0
#define VM_MINOR_VERSION 1

#include <iostream>
#include <vector>
#include <stack>


#include "src/common.hpp"
#include "src/chunk.hpp"
#include "src/opcode.hpp"
#include "src/object.hpp"
#include "src/instruction.hpp"
#include "src/value.hpp"


void vm_print_version();


struct CallFrame {

    CallFrame(ObjLambda* fn, Value* stack, u32 ip = 0);

    Instruction next();

    ObjLambda* fn;            // the function being called
    u32 ip;                     // the instruction pointer within the function
    std::vector<Value> locals;  // all local variables and parameters
    Value* fp;                  // frame pointer
};

enum ExecutionStatus {
    Execution_Complete,
    Execution_Error
};

class Execution {
public:
    Execution();

    ExecutionStatus run(ObjLambda* lambda);

    ExecutionStatus run();

    // Value run(Chunk* chunk, u32 ip);
    u32 add_constant(const Value& val);

    Value& peek(int dist = 0);

    u32 add_global(const Value& val);

private:
    Value pop();

    void push(const Value& val);

    Value* grow_stack();

    void call(ObjLambda* lambda, const std::vector<Value>& params);
private:
    Chunk* chunk;
    // u32 ip;


    u32 size;
    u32 cap;
    Value* stack;
    Value* topStack;
    CallFrame* activeFrame;
    std::vector<Value> globals;
    std::stack<CallFrame*> frames;
};

// the thought was this could be a high level interaction with
// the exectution engine that allows for the creation of
// objects and then run them. This should eventually be the
// interface a language compiler/interpreter would use to execute.
class VM {
public:
    //
private:
    Execution exc;
};

#include <iostream>
#include "vm.hpp"

int main() {
    vm_print_version();


    /*
    bar(x, y) -> f32 {
        return x ** y // i think
    }
    main() {
        bar(3.)
    }
    */
    Execution exc;
    Chunk chunk;
    chunk.add_constant(Value((i64) 1));
    chunk.add_constant(Value((f64) 3));
    chunk.add_constant(Value((f64) 3));
    chunk.write_instruction(Instruction(Load_Global, 0, 0), 0);
    chunk.write_instruction(Instruction(Load_Const, 0, 2), 0);
    chunk.write_instruction(Instruction(Load_Const, 0, 1), 0);
    chunk.write_instruction(Instruction(Call, 0, 2), 0);
    chunk.write_instruction(Instruction(Return, 0, 0), 1);

    Chunk barchunk;
    // barchunk.add_constant(Value(4.0));
    barchunk.write_instruction(Instruction(Load_Local, 0, 0), 10);
    barchunk.write_instruction(Instruction(Load_Local, 0, 1), 10);
    barchunk.write_instruction(Instruction(Pow_Double), 10);
    barchunk.write_instruction(Instruction(Return, 0, 0), 10);

    // Chunk fchunk;
    // auto findex = fchunk.add_constant(Value(1.));
    // auto frindex = chunk.add_constant(Value((i64) 1));
    // fchunk.write_instruction(Instruction(Load_Const, 0, findex), 10);
    // fchunk.write_instruction(Instruction(Return, frindex), 10);
    //
    // auto index = chunk.add_constant(Value(2.0));
    // auto index2 = chunk.add_constant(Value(10.0));
    // auto rindex = chunk.add_constant(Value((i64) 0));
    // auto fname = chunk.add_constant(Value(new ObjLambda(new ObjString("foo"), fchunk, 0)));
    //
    // chunk.write_instruction(Instruction(Load_Const, 0, index), 1);
    // chunk.write_instruction(Instruction(Load_Const, 0, index), 1);
    // chunk.write_instruction(Instruction(Pow_Double), 1);
    // chunk.write_instruction(Instruction(Load_Const, 0, index2), 1);
    // chunk.write_instruction(Instruction(Multi_Double), 1);
    // chunk.write_instruction(Instruction(Call, 0, fname), 2);
    // chunk.write_instruction(Instruction(Create_Local), 2);
    // chunk.write_instruction(Instruction(Load_Local, 0, 0), 3); // this puts ontop of the stack so it should be show as the result of th bytecode.
    // chunk.write_instruction(Instruction(Return, rindex), 3);

    auto lambda = ObjLambda(new ObjString("main"), chunk, 0);
    auto bar = new ObjLambda(new ObjString("bar"), barchunk, 2);
    exc.add_global(Value(bar));

    auto status = exc.run(&lambda);

    if(status == Execution_Complete) {
        std::cout << "Value: " << exc.peek() << std::endl;
    }
    else if(status == Execution_Error){

    }



    return 0;
}

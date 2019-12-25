//
// Created by Andrew Bregger on 2019-02-18.
//


#pragma once

enum Backend {
    Bytecode,
    LLVM, // not implemented
    X64, // not implemented

};

class CodeGen {
public:
    explicit CodeGen(Backend bck);
private:
    Backend backend;
};


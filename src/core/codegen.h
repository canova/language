#ifndef codegen_h
#define codegen_h

#include <stack>
#include <typeinfo>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include "../logger.h"

using namespace llvm;

static LLVMContext TheContext;

class NBlock;

class CodeGenBlock {
public:
    BasicBlock *block;
    Value *returnValue;
    std::map<std::string, Value*> locals;
};

class CodeGenContext {
    std::stack<CodeGenBlock *> blocks;
    Function *mainFunction;

public:
    Value *cObject;
    Module *module;
    Function *objallocFunction;
    Function *putSlotFunction;
    Function *getSlotFunction;
    Function *newobjFunction;

    CodeGenContext() {
        module = new Module("main.ll", TheContext);
    }

    StructType *addStructType(char *name, size_t numArgs, ...);
    FunctionType *functionType(Type* retType, bool varargs, size_t numArgs, ...);
    Function *addExternalFunction(char *name, FunctionType *ftype);
    Function *addFunction(char *name, FunctionType *ftype, void (^block)(BasicBlock *));

    void generateCode(NBlock& root);
    void runCode();

    std::map<std::string, Value*>& locals() {
        return blocks.top()->locals;
    }

    BasicBlock *currentBlock() {
        return blocks.top()->block;
    }

    void pushBlock(BasicBlock *block) {
        blocks.push(new CodeGenBlock());
        blocks.top()->returnValue = NULL;
        blocks.top()->block = block;
    }

    void popBlock() {
        CodeGenBlock *top = blocks.top();
        blocks.pop();
        delete top;
    }

    void setCurrentReturnValue(Value *value) {
        blocks.top()->returnValue = value;
    }

    Value *getCurrentReturnValue() {
        return blocks.top()->returnValue;
    }
};

#endif // codegen_h

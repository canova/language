#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;


llvm::Function* createPrintfFunction(CodeGenContext& context)
{
    std::vector<llvm::Type*> printf_arg_types;
    printf_arg_types.push_back(llvm::Type::getInt8PtrTy(context.module->getContext())); //char*

    llvm::FunctionType* printf_type =
        llvm::FunctionType::get(
            llvm::Type::getInt64Ty(context.module->getContext()), printf_arg_types, true);

    llvm::Function *func = llvm::Function::Create(
                printf_type, llvm::Function::ExternalLinkage,
                llvm::Twine("printf"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

void createEchoIntegerFunction(CodeGenContext& context, llvm::Function* printfFn)
{
    LOG(LogLevel::Verbose, "Creating sayi_yaz function");
    std::vector<llvm::Type*> echo_arg_types;
    echo_arg_types.push_back(llvm::Type::getInt64Ty(context.module->getContext()));

    llvm::FunctionType* echo_type =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(context.module->getContext()), echo_arg_types, false);

    llvm::Function *func = llvm::Function::Create(
                echo_type, llvm::Function::InternalLinkage,
                llvm::Twine("sayi_yaz"),
                context.module
           );
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(context.module->getContext(), "entry", func, 0);
    context.pushBlock(bblock);

    const char *constValue = "%d\n";
    llvm::Constant *format_const = llvm::ConstantDataArray::getString(context.module->getContext(), constValue);
    llvm::GlobalVariable *var =
        new llvm::GlobalVariable(
            *context.module, llvm::ArrayType::get(llvm::IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
            true, llvm::GlobalValue::PrivateLinkage, format_const, ".str");
    llvm::Constant *zero =
        llvm::Constant::getNullValue(llvm::IntegerType::getInt64Ty(context.module->getContext()));

    std::vector<llvm::Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    llvm::Constant *var_ref = llvm::ConstantExpr::getGetElementPtr(
    llvm::ArrayType::get(llvm::IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
        var, indices);

    std::vector<Value*> args;
    args.push_back(var_ref);

    Function::arg_iterator argsValues = func->arg_begin();
    Value* toPrint = &*argsValues++;
    toPrint->setName("toPrint");
    args.push_back(toPrint);

    CallInst *call = CallInst::Create(printfFn, makeArrayRef(args), "", bblock);
    ReturnInst::Create(context.module->getContext(), bblock);
    context.popBlock();
}

void createEchoStringFunction(CodeGenContext& context, llvm::Function* printfFn)
{
    LOG(LogLevel::Verbose, "Creating yazi_yaz function");
    std::vector<llvm::Type*> echo_arg_types;
    echo_arg_types.push_back(llvm::Type::getInt8PtrTy(context.module->getContext()));

    llvm::FunctionType* echo_type =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(context.module->getContext()), echo_arg_types, false);

    llvm::Function *func = llvm::Function::Create(
                echo_type, llvm::Function::InternalLinkage,
                llvm::Twine("yazi_yaz"),
                context.module
           );
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(context.module->getContext(), "entry", func, 0);
    context.pushBlock(bblock);

    const char *constValue = "%s\n";
    llvm::Constant *format_const = llvm::ConstantDataArray::getString(context.module->getContext(), constValue);
    llvm::GlobalVariable *var =
        new llvm::GlobalVariable(
            *context.module, llvm::ArrayType::get(llvm::IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
            true, llvm::GlobalValue::PrivateLinkage, format_const, ".str");
    llvm::Constant *zero =
        llvm::Constant::getNullValue(llvm::IntegerType::getInt64Ty(context.module->getContext()));

    std::vector<llvm::Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    llvm::Constant *var_ref = llvm::ConstantExpr::getGetElementPtr(
    llvm::ArrayType::get(llvm::IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
        var, indices);

    std::vector<Value*> args;
    args.push_back(var_ref);

    Function::arg_iterator argsValues = func->arg_begin();
    Value* toPrint = &*argsValues++;
    toPrint->setName("toPrint");
    args.push_back(toPrint);

    CallInst *call = CallInst::Create(printfFn, makeArrayRef(args), "", bblock);
    ReturnInst::Create(context.module->getContext(), bblock);
    context.popBlock();
}

void createCoreFunctions(CodeGenContext& context){
    LOG(LogLevel::Verbose, "Creating core functions");
    llvm::Function* printfFn = createPrintfFunction(context);
    createEchoIntegerFunction(context, printfFn);
    createEchoStringFunction(context, printfFn);
}

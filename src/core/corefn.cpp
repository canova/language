#include <iostream>
#include "codegen.h"

Function* createPrintfFunction(CodeGenContext& context)
{
    vector<Type*> printf_arg_types;
    printf_arg_types.push_back(Type::getInt8PtrTy(context.module->getContext())); //char*

    FunctionType* printf_type =
        FunctionType::get(
            Type::getInt64Ty(context.module->getContext()), printf_arg_types, true);

    Function *func = Function::Create(
                printf_type, Function::ExternalLinkage,
                Twine("printf"),
                context.module
           );
    func->setCallingConv(CallingConv::C);
    return func;
}

void createEchoIntegerFunction(CodeGenContext& context, Function* printfFn)
{
    LOG(LogLevel::Verbose, "Creating sayi_yaz function");
    vector<Type*> echo_arg_types;
    echo_arg_types.push_back(Type::getInt64Ty(context.module->getContext()));

    FunctionType* echo_type =
        FunctionType::get(
            Type::getVoidTy(context.module->getContext()), echo_arg_types, false);

    Function *func = Function::Create(
                echo_type, Function::InternalLinkage,
                Twine("sayi_yaz"),
                context.module
           );
    BasicBlock *bblock = BasicBlock::Create(context.module->getContext(), "entry", func, 0);
    context.pushBlock(bblock);

    const char *constValue = "%d\n";
    Constant *format_const = ConstantDataArray::getString(context.module->getContext(), constValue);
    GlobalVariable *var =
        new GlobalVariable(
            *context.module, ArrayType::get(IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
            true, GlobalValue::PrivateLinkage, format_const, ".str");
    Constant *zero =
        Constant::getNullValue(IntegerType::getInt64Ty(context.module->getContext()));

    vector<Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    Constant *var_ref = ConstantExpr::getGetElementPtr(
    ArrayType::get(IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
        var, indices);

    vector<Value*> args;
    args.push_back(var_ref);

    Function::arg_iterator argsValues = func->arg_begin();
    Value* toPrint = &*argsValues++;
    toPrint->setName("toPrint");
    args.push_back(toPrint);

    CallInst *call = CallInst::Create(printfFn, makeArrayRef(args), "", bblock);
    ReturnInst::Create(context.module->getContext(), bblock);
    context.popBlock();
}

void createEchoStringFunction(CodeGenContext& context, Function* printfFn)
{
    LOG(LogLevel::Verbose, "Creating yazi_yaz function");
    vector<Type*> echo_arg_types;
    echo_arg_types.push_back(Type::getInt8PtrTy(context.module->getContext()));

    FunctionType* echo_type =
        FunctionType::get(
            Type::getVoidTy(context.module->getContext()), echo_arg_types, false);

    Function *func = Function::Create(
                echo_type, Function::InternalLinkage,
                Twine("yazi_yaz"),
                context.module
           );
    BasicBlock *bblock = BasicBlock::Create(context.module->getContext(), "entry", func, 0);
    context.pushBlock(bblock);

    const char *constValue = "%s\n";
    Constant *format_const = ConstantDataArray::getString(context.module->getContext(), constValue);
    GlobalVariable *var =
        new GlobalVariable(
            *context.module, ArrayType::get(IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
            true, GlobalValue::PrivateLinkage, format_const, ".str");
    Constant *zero =
        Constant::getNullValue(IntegerType::getInt64Ty(context.module->getContext()));

    vector<Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    Constant *var_ref = ConstantExpr::getGetElementPtr(
    ArrayType::get(IntegerType::get(context.module->getContext(), 8), strlen(constValue) + 1),
        var, indices);

    vector<Value*> args;
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
    Function* printfFn = createPrintfFunction(context);
    createEchoIntegerFunction(context, printfFn);
    createEchoStringFunction(context, printfFn);
}

#include "node.h"
#include "codegen.h"
#include "../grammar/parser.hpp"
#include "types.h"
#include <stdarg.h>
#include <string>

using namespace std;

static StructType *ObjectType;
static StructType *StringType;
static PointerType *ObjectType_p;
static PointerType *StringType_p;

static PointerType *GenericPointerType = PointerType::get(Type::getInt64Ty(TheContext), 0);

mObject (*objalloc)() = NULL;

StructType* CodeGenContext::addStructType(char *name, size_t numArgs, ...)
{
    LOG(LogLevel::Verbose, "addStructType");
    vector<Type *> fields;
    StructType *fwdType = StructType::create(TheContext, name);
    fields.push_back(PointerType::getUnqual(fwdType));
    fields.push_back(Type::getInt64Ty(TheContext));

    va_list list;
    va_start(list, numArgs);
    for (int i = 0; i < numArgs; i++) {
        fields.push_back(va_arg(list, Type*));
    }
    va_end(list);

    fwdType->setBody(makeArrayRef(fields));
    return fwdType;
}

FunctionType* CodeGenContext::functionType(Type* retType, bool varargs, size_t numArgs, ...)
{
    vector<Type*> args;
    va_list list;
    va_start(list, numArgs);
    for (int i = 0; i < numArgs; i++) {
        args.push_back(va_arg(list, Type*));
    }
    va_end(list);

    return FunctionType::get(retType, makeArrayRef(args), varargs);
}

Function* CodeGenContext::addExternalFunction(char *name, FunctionType *fType)
{
    Function *f = Function::Create(fType, GlobalValue::ExternalLinkage, name, module);
    f->setCallingConv(CallingConv::C);
    return f;
}

Function* CodeGenContext::addFunction(char *name, FunctionType *fType, void (^block)(BasicBlock *))
{
    Function *f = Function::Create(fType, GlobalValue::InternalLinkage, name, module);
    f->setCallingConv(CallingConv::C);
    if (block) block(BasicBlock::Create(TheContext, (char *) "entry", f, 0));
    return f;
}

/* Compile the AST into a module */
void CodeGenContext::generateCode(NBlock& root)
{
    LOG(LogLevel::Debug, "Generating code...");

    ObjectType = addStructType((char *) "mObject", 1, GenericPointerType);
    ObjectType_p = PointerType::getUnqual(ObjectType);
    StringType = addStructType((char *) "string", 3, GenericPointerType, GenericPointerType, Type::getInt64Ty(TheContext));
    StringType_p = PointerType::getUnqual(StringType);

    /* Create objalloc function */
    objallocFunction = addFunction((char *) "objalloc", functionType(ObjectType_p, false, 0), ^(BasicBlock *blk) {
        Constant* allocsize = ConstantExpr::getSizeOf(Type::getInt64Ty(TheContext));
        Value* a = CallInst::CreateMalloc(blk, ObjectType,
                                          Type::getInt64Ty(TheContext),
                                          allocsize);
        ReturnInst::Create(TheContext, a, blk); // FIXME: IS MALLOC TRUE?
    });

    /* Create refs to putSlot, getSlot and newobj */
    putSlotFunction = addExternalFunction((char *) "putSlot",
        functionType(Type::getVoidTy(TheContext), false, 3, ObjectType_p, GenericPointerType, ObjectType_p));
    getSlotFunction = addExternalFunction((char *) "getSlot",
        functionType(ObjectType_p, false, 3, ObjectType_p, GenericPointerType, Type::getInt64Ty(TheContext)));
    newobjFunction = addExternalFunction((char *) "newobj",
        functionType(ObjectType_p, false, 1, ObjectType_p));

    /* Create the top level interpreter function to call as entry */
    vector<Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(TheContext), makeArrayRef(argTypes), false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
    BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", mainFunction, 0);

    /* Push a new variable/block context */
    pushBlock(bblock);
    cObject = new GlobalVariable(*module, ObjectType, true,
        GlobalValue::InternalLinkage, 0, "class.Object");
    root.codeGen(*this); /* emit bytecode for the toplevel block */
    ReturnInst::Create(TheContext, bblock);
    popBlock();

    /* Print the bytecode in a human-readable format
       to see if our program compiled properly
       Comment these lines after debugging.
     */
    LOG(LogLevel::Debug, "Code is generated.");
    //module->dump();
    LOG(LogLevel::Verbose, "Dump ends.");
}

/* Executes the AST by running the main function */
void CodeGenContext::runCode() {
    LOG(LogLevel::Debug, "Running code...");
    string error;
    ExecutionEngine *ee = EngineBuilder(unique_ptr<Module>(module)).setErrorStr(&error).create();
    objalloc = (mObject (*)())ee->getPointerToFunction(objallocFunction);
    ee->finalizeObject();

    const vector<string> argList;
    ee->runFunctionAsMain(mainFunction, argList, 0);
    LOG(LogLevel::Info, "\033[0;32mCode was run.\x1b[0m");

    if (error.length() > 0)
        LOG(LogLevel::Error, "Error exist: " + error);
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const VariableType type)
{
    switch(type) {
        case VariableType::Integer:
            return Type::getInt64Ty(TheContext);
        case VariableType::Double:
            return Type::getDoubleTy(TheContext);
        case VariableType::String:
        case VariableType::Object:
            return ObjectType_p;
        default:
            return Type::getVoidTy(TheContext);
    }
}

static Value* getStringConstant(const string& str, CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "getStringConstant str: " + str);
    Constant *n = ConstantDataArray::getString(TheContext, str.c_str(), true);
    GlobalVariable *g = new GlobalVariable(*context.module, n->getType(), true,
        GlobalValue::InternalLinkage, 0, str.c_str());
    g->setInitializer(n);
    vector<Constant*> ptr;
    ptr.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 0));
    ptr.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 0));
    return ConstantExpr::getGetElementPtr(StringType_p, g, ptr[0], ptr.size());
}

static Value* resolveReference(NReference& ref, CodeGenContext& context, bool ignoreLast = false)
{
    Value *curValue = ref.refs.front()->codeGen(context);
    IdentifierList::const_iterator it;
    for (it = ref.refs.begin() + 1; it != ref.refs.end(); it++) {
        NIdentifier& ident = **it;
        if (ignoreLast && it == ref.refs.end() - 1) return curValue;

        LOG(LogLevel::Verbose, "Next ident: " + ident.name);
        Value *ch = getStringConstant(ident.name, context);

        vector<Value*> params;
        params.push_back(curValue);
        params.push_back(ch);
        params.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 1));
        LOG(LogLevel::Verbose, "About to call " + string(typeid(curValue).name()) + "::" + typeid(ch).name());
        CallInst *call = CallInst::Create(context.getSlotFunction, makeArrayRef(params), "");
        call->setCallingConv(CallingConv::C);
        curValue = call;
    }
    return curValue;
}

/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Creating integer: " + to_string(value));
    return ConstantInt::get(Type::getInt64Ty(TheContext), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Creating double: " + to_string(value));
    return ConstantFP::get(Type::getDoubleTy(TheContext), value);
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Creating identifier: " + name);
    if (name.compare("null") == 0) {
        return ConstantPointerNull::get(ObjectType_p);
    }

    if (context.locals().find(name) == context.locals().end()) {
        LOG(LogLevel::Verbose, "Instantiating object: " + name);
        vector<Value*> args;
        args.push_back(ConstantPointerNull::get(ObjectType_p));
        CallInst *call = CallInst::Create(context.newobjFunction, makeArrayRef(args), "");
        return context.locals()[name] = call;
    }

    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* NString::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Creating string: " + value);
    Constant *format_const = ConstantDataArray::getString(TheContext, value);
    GlobalVariable *var =
        new GlobalVariable(
            *context.module, ArrayType::get(IntegerType::get(TheContext, 8), value.length() + 1),
            true, GlobalValue::PrivateLinkage, format_const, ".str");
    // Old NString codegen
    //args.push_back(ConstantPointerNull::get(ObjectType_p));
    //return CallInst::Create(context.newobjFunction, makeArrayRef(args), "");
    return var;
}

Value* NReference::codeGen(CodeGenContext& context)
{
    if (refs.size() == 1) {
        NIdentifier *ident = refs.front();
        LOG(LogLevel::Verbose, "Creating reference 1: " + ident->name);
        return ident->codeGen(context);
    }

    LOG(LogLevel::Verbose, "Creating reference 2 ");
    return resolveReference(*this, context);
}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
    NIdentifier& id = *ref.refs.front();
    Function *function = context.module->getFunction(id.name.c_str());
    if (function == NULL) {
        LOG(LogLevel::Error, "No such function " + id.name);
    }
    vector<Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        args.push_back((**it).codeGen(context));
    }
    CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
    LOG(LogLevel::Verbose, "Creating method call: " + id.name);
    return call;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Creating binary operation " + to_string(op));
    Instruction::BinaryOps instr;
    switch (op) {
        case TPLUS:     instr = Instruction::Add; goto math;
        case TMINUS:    instr = Instruction::Sub; goto math;
        case TMUL:      instr = Instruction::Mul; goto math;
        case TDIV:      instr = Instruction::SDiv; goto math;

        /* TODO comparison */
    }

    return NULL;
math:
    return BinaryOperator::Create(instr, lhs.codeGen(context),
        rhs.codeGen(context), "", context.currentBlock());
}

Value* NAssignment::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Debug, "Creating assignment");
    if (lhs.refs.size() == 1) {
        LOG(LogLevel::Verbose, "Assignment refs.size == 1");
        return new StoreInst(rhs.codeGen(context), context.locals()[lhs.refs.front()->name], false, context.currentBlock());
    } else {
        LOG(LogLevel::Verbose, "Assignment refs.size != 1");
        Value *value = resolveReference(lhs, context, true);
        Value *sym = getStringConstant(lhs.refs.back()->name, context);

        vector<Value*> params;
        params.push_back(value);
        params.push_back(sym);
        params.push_back(rhs.codeGen(context));
        CallInst *call = CallInst::Create(context.putSlotFunction, makeArrayRef(params), "");
        call->setCallingConv(CallingConv::C);
        return call;
    }
}

Value* NBlock::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    Value *last = NULL;
    for (it = statements.begin(); it != statements.end(); it++) {
        LOG(LogLevel::Verbose, "Generating code for block " + string(typeid(**it).name()));
        last = (**it).codeGen(context);
    }

    LOG(LogLevel::Verbose, "Creating block");
    return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Generating code for expression" + string(typeid(expression).name()));
    return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Generating return code for" + string(typeid(expression).name()));
    Value *returnValue = expression.codeGen(context);
    context.setCurrentReturnValue(returnValue);
    return returnValue;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, "Creating variable declaration " + to_string(type) + " " + id.name);
    AllocaInst *alloc = new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
    context.locals()[id.name] = alloc;
    if (assignmentExpr != NULL) {
        NReference ref(id);
        NAssignment assn(ref, *assignmentExpr);
        assn.codeGen(context);
    }
    return alloc;
}

Value* NExternDeclaration::codeGen(CodeGenContext& context)
{
    vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
    return function;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
    LOG(LogLevel::Verbose, id.name);
    vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
    BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", function, 0);

    context.pushBlock(bblock);

    Function::arg_iterator argsValues = function->arg_begin();
    Value* argumentValue;

    for (it = arguments.begin(); it != arguments.end(); it++) {
        (**it).codeGen(context);

        argumentValue = &*argsValues++;
        argumentValue->setName((*it)->id.name.c_str());
        StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->id.name], false, bblock);
    }

    block.codeGen(context);
    ReturnInst::Create(TheContext, context.getCurrentReturnValue(), bblock);

    context.popBlock();
    LOG(LogLevel::Verbose, "Creating function: " + id.name);
    return function;
}

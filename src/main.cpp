#include <iostream>
#include <locale.h>
#include "core/codegen.h"
#include "core/node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "Turkish");
    yyparse();
    LOG(LogLevel::Error, "Main function");

    // http://comments.gmane.org/gmane.comp.compilers.llvm.devel/33877
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    CodeGenContext context;
    createCoreFunctions(context);
    context.generateCode(*programBlock);
    context.runCode();

    return 0;
}

#include <iostream>
#include <fstream>
#include <memory>

// ANTLR
#include "LumaLexer.h"
#include "LumaParser.h"

// LLVM
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Support/TargetSelect.h"


// AST
#include "parser/AstBuilder.h"
#include "ast/Statement.h"

// CodeGen
#include "codegen/CodeGen.h"
// ErrorHandler
#include "common/Global.h"

using namespace antlr4;

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Usage: ./Luma <source_file>\n";
        return 1;
    }
    
    std::ifstream file(argv[1]);
    if(!file){
        std::cerr << "Could not open file: " << argv[1] << "\n";
        return 1;
    }
    
    ANTLRInputStream inputStream(file);
    Luma::LumaLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    Luma::LumaParser parser(&tokens);
    
    tree::ParseTree* tree = parser.program();
    
    AstBuilder astBuilder;
    antlrcpp::Any astRootAny = astBuilder.visit(tree);

    if(astRootAny.has_value()){
        try{
            auto progNode = std::any_cast<std::shared_ptr<ProgramNode>>(astRootAny);
            CodeGen codeGenerator;
            codeGenerator.generate(progNode.get());
            auto context = std::make_unique<llvm::LLVMContext>();
            llvm::Module* module = codeGenerator.getModule();
            if (llvm::verifyModule(*module, &llvm::errs())) {
                std::cerr << "LLVM Module Verification Failed!\n";
                return 1;
            }

            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();

            auto jit = llvm::orc::LLJITBuilder().create();
            if(!jit){
                std::cerr << "Failed to create LLJIT instance: " << toString(jit.takeError()) << "\n";
                return 1;
            }

            auto threadSafeModule = llvm::orc::ThreadSafeModule(codeGenerator.releaseModule(), std::move(context));
            
            auto err = (*jit)->addIRModule(std::move(threadSafeModule));
            if(err){
                std::cerr << "Failed to add IR module: " << toString(std::move(err)) << "\n";
                return 1;
            }

            auto mainFuncSym = (*jit)->lookup("main");
            if(!mainFuncSym){
                std::cerr << "Could not find main function: " << toString(mainFuncSym.takeError()) << "\n";
                return 1;
            }

            auto* mainFunc = mainFuncSym->toPtr<int()>();
            if(!mainFunc) { return 1; }
            mainFunc();
            errorHandler.errorReg("Compile Finished!", 2);
            errorHandler.printAllErrors();
        }catch (const std::bad_any_cast& e) {
            std::cerr << "AST construction failed: returned type is not ProgramNode.\n";
        }
    }else{
        std::cout << "Failed to construct AST (visitor returned empty).\n";
    }
    return 0;
}
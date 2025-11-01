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
#include "llvm/IR/LLVMContext.h"

// AST
#include "parser/AstBuilder.h"
#include "ast/Statement.h"

// CodeGen
#include "codegen/CodeGen.h"

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
    
    std::cout << "--- Parse Tree ---\n";
    std::cout << tree->toStringTree(&parser) << "\n\n";

    // --- AST構築 ---
    std::cout << "--- AST Construction Trace ---\n";
    AstBuilder astBuilder;
    antlrcpp::Any astRootAny = astBuilder.visit(tree); // ここでビジターが動く

    std::cout << "\n--- AST Construction Result ---\n";
    if(astRootAny.has_value()){
        try{
            auto progNode = std::any_cast<std::shared_ptr<ProgramNode>>(astRootAny);
            // LLVM IR生成
            std::cout << "\n--- Generated LLVM IR ---\n";
            CodeGen codeGenerator;
            codeGenerator.generate(progNode.get()); // コード生成を開始
            llvm::Module* module = codeGenerator.getModule();
            if (llvm::verifyModule(*codeGenerator.getModule(), &llvm::errs())) {
                std::cerr << "LLVM Module Verification Failed!\n";
            }
            // 生成されたIRをコンソールに出力
            std::cout << "\n--- Generated LLVM IR ---\n";
            if(module){
                module->print(llvm::errs(), nullptr);
            }
        }catch (const std::bad_any_cast& e) {
            std::cerr << "AST construction failed: returned type is not ProgramNode.\n";
        }
    }else{
        std::cout << "Failed to construct AST (visitor returned empty).\n";
    }
    return 0;
}
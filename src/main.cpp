#include <iostream>
#include <fstream>
#include <memory>
#include <any>

// ANTLR
#include "antlr4-runtime.h"
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
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

// Local
#include "common/ErrorDef.h"
#include "common/ErrorHandler.h"
#include "parser/AstBuilder.h"
#include "ast/Statement.h"
#include "codegen/CodeGen.h"
#include "semantic/SemanticAnalysis.h"
#include "common/Global.h"
#include "llvmgen/LLVMGen.h"

// MIRGen
#include "mirgen/MIRGen.h" // MIRGen のヘッダをインクルード

using namespace antlr4;

int main(int argc, char* argv[]){
    std::string sourceFile;
    Language lang = Language::EN;
    bool debug_ast_print = false; // ASTダンプフラグ (ユーザーの変数名に合わせる)
    bool dbg_mir_print = false; // MIRダンプフラグ (新規追加)

    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];
        if(arg == "-ja") lang = Language::JA;
        else if(arg == "-en") lang = Language::EN;
        else if(arg == "-dbg-ast-print") debug_ast_print = true; // フラグをセット
        else if(arg == "-dbg-mir-print") dbg_mir_print = true; // フラグをセット (新規追加)
        else if(arg == "-debug-ast-print") std::cerr << "Correct: -dbg-ast-print" << "\n";
        else if(arg == "-debug-mir-print") std::cerr << "Correct: -dbg-mir-print" << "\n";
        else if(sourceFile.empty()) sourceFile = arg;
        else{
            std::cerr << "Too many source files specified.\n";
            return 1;
        }
    }

    if(sourceFile.empty()){
        std::cerr << "Usage: ./Luma [-ja|-en] [-dbg-ast-print] [-dbg-mir-print] <source_file>\n"; // Usageメッセージ更新
        return 1;
    }

    errorHandler.setLang(lang);
    std::ifstream file(sourceFile);
    if(!file){
        std::cerr << "Could not open file: " << sourceFile << "\n";
        return 1;
    }
    
    ANTLRInputStream inputStream(file);
    Luma::LumaLexer lexer(&inputStream);
    CommonTokenStream tokens(&lexer);
    Luma::LumaParser parser(&tokens);
    
    tree::ParseTree* tree = parser.program();
    
    AstBuilder astBuilder;
    std::shared_ptr<ProgramNode> programNode;

    try {
        auto anyNode = tree->accept(&astBuilder);
        if (anyNode.has_value()) {
            programNode = std::any_cast<std::shared_ptr<ProgramNode>>(anyNode);
        }
    } catch (const std::bad_any_cast& e) {
        std::cerr << "AST construction failed: " << e.what() << std::endl;
        return 1;
    }
    
    if (programNode) {
        // ASTのダンプ (フラグが立っている場合のみ)
        if(debug_ast_print){
            std::cout << "--- AST Dump (Before Semantic Analysis) ---" << std::endl;
            programNode->dump();
            std::cout << "-------------------------------------------\n" << std::endl;
        }

        // セマンティック解析
        SemanticAnalysis semanticAnalysis;
        semanticAnalysis.visit(programNode.get());
        if (semanticAnalysis.hasErrors()) {
            errorHandler.printAllErrors();
            return 1;
        }

        // ASTのダンプ(セマンティック解析後)
        if(debug_ast_print){
            std::cout << "--- AST Dump (After Semantic Analysis) ---" << std::endl;
            programNode->dump();
            std::cout << "------------------------------------------\n" << std::endl;
        }

        // MIR生成 (新規追加)
        MIRGen mirGen(semanticAnalysis); // MIRGen のインスタンス化
        std::unique_ptr<MIRModule> mirModule = mirGen.generate(programNode); // MIRを生成

        // MIRのダンプ (フラグが立っている場合のみ) (新規追加)
        if (dbg_mir_print && mirModule) {
            std::cout << "--- MIR Dump ---" << std::endl;
            mirModule->dump(std::cout);
            std::cout << "----------------" << std::endl;
        }

        // コード生成 (CodeGen)
        // CodeGenはまだASTから直接LLVM IRを生成しているので、
        // ここではMIRは使わずにASTを渡す。
        // 将来的にはMIRからLLVM IRを生成するように変更する。
        // CodeGen codeGen(semanticAnalysis);
        // codeGen.generate(programNode.get());
        
        // auto context = std::make_unique<llvm::LLVMContext>();
        // std::unique_ptr<llvm::Module> module = codeGen.releaseModule();
        LLVMGen llvmGen(semanticAnalysis);
        llvmGen.generate(mirModule.get());
        auto context = std::make_unique<llvm::LLVMContext>();
        std::unique_ptr<llvm::Module> module = llvmGen.releaseModule();

        llvm::errs() << "--- LLVM IR Dump (Before Verification) ---\n";
        module->print(llvm::errs(), nullptr);
        llvm::errs() << "------------------------------------------\n";
        errorHandler.printAllErrors();

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

        auto threadSafeModule = llvm::orc::ThreadSafeModule(std::move(module), std::move(context));
        
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
        
        // IRをファイルに出力
        std::error_code EC;
        llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);
        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();
            return 1;
        }
        // JITに渡す前にModuleの参照を保持しておく必要がある場合、ここでprintする
        // ただし、releaseModuleで所有権は移譲済みなので、再度moduleポインタを使う場合は注意が必要
        // ここではJIT実行前のIRを確認する目的としておく
        // (*jit)->getExecutionSession().getIRModule(mainFuncSym->getJITDylib()).getModule()->print(dest, nullptr);


        auto* mainFunc = mainFuncSym->toPtr<int()>();
        if(!mainFunc) { return 1; }
        mainFunc();
        
        errorHandler.errorReg("Semantic Analysis Finished!", 2);
        errorHandler.errorReg("Compile Finished!", 2);
        errorHandler.printAllErrors();

    } else {
        std::cout << "Failed to construct AST (visitor returned empty).\n";
        return 1;
    }

    return 0;
}
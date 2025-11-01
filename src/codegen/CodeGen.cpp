#include <iostream>
#include "CodeGen.h"
#include "ast/Statement.h"
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

CodeGen::CodeGen(){
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("LumaModule", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

void CodeGen::generate(ProgramNode* root){
    visit(root);
}

llvm::Module* CodeGen::getModule(){
    return module.get();
}

// Program
void CodeGen::visit(ProgramNode* node){
    // main関数を作成
    auto funcType = llvm::FunctionType::get(builder->getInt32Ty(), false); // main関数の型(i32 main())を定義
    auto mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module.get()); // moduleにmain関数を作成
    // IRBuilderの挿入ポイントに設定
    auto entryBlock = llvm::BasicBlock::Create(*context, "entry", mainFunc);
    builder->SetInsertPoint(entryBlock);
    // プログラム内の各文をvisit
    for(const auto& stmt : node->statements) visit(stmt.get());
    // return 0を追加
    builder->CreateRet(builder->getInt32(0));
    if (llvm::verifyFunction(*mainFunc, &llvm::errs())) {
        std::cerr << "LLVM Function Verification Failed!\n";
    } else {
        std::cout << "LLVM Function Verification Succeeded.\n";
    }
}

// Statement
void CodeGen::visit(StatementNode* node){
    if(auto varNode = dynamic_cast<VarDeclNode*>(node)){
        visit(varNode);
    }else{
        std::cerr << "Error: unknown statement";
        return;
    }
}

// varDecl 変数宣言
llvm::Value* CodeGen::visit(VarDeclNode* node){
    std::string varName = node->varName; // 変数名
    auto type = builder->getInt32Ty(); // 型 TODO: 複数型対応
    auto allocaInst = builder->CreateAlloca(type, nullptr, varName); // メモリ確保
    namedValues[node->varName] = allocaInst;
    return 0;
}
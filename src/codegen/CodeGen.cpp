#include <iostream>
#include "CodeGen.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/BasicBlock.h"

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
    }else if(auto assignmentNode = dynamic_cast<AssignmentNode*>(node)){
        visit(assignmentNode);
    }else if(auto ifNode = dynamic_cast<IfNode*>(node)){
        visit(ifNode);
    }else{
        std::cerr << "Error: unknown statement.\n";
        return;
    }
}

// Block
void CodeGen::visit(BlockNode* node){
    for(const auto& stmt : node->statements){
        visit(stmt.get());
    }
}

// Expr
llvm::Value* CodeGen::visit(ExprNode *node){
    // dynamic_castでどれにキャストできるか分ける
    if(auto cnode = dynamic_cast<NumberLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<BinaryOpNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VariableRefNode*>(node)) return visit(cnode);
    std::cerr << "Error: ExprNode couldn't cast.\n";
    return nullptr;
}

// varDecl 変数宣言
llvm::Value* CodeGen::visit(VarDeclNode* node){
    std::string varName = node->varName; // 変数名
    auto type = builder->getInt32Ty(); // 型 TODO: 複数型対応
    auto allocaInst = builder->CreateAlloca(type, nullptr, varName); // メモリ確保
    namedValues[node->varName] = allocaInst;
    if(node->initializer){
        llvm::Value* initVal = visit(node->initializer.get());
        builder->CreateStore(initVal, allocaInst);
    }
    return 0;
}

// assignment 代入
llvm::Value* CodeGen::visit(AssignmentNode *node){
    std::string varName = node->varName;
    auto it = namedValues.find(varName);
    if(it == namedValues.end()){
        std::cerr << "Error: Assignment to undeclared variable '" << varName << "'\n";
        return nullptr;
    }
    auto address = namedValues[varName];
    llvm::Value* val = visit(node->value.get());
    if(!val){
        std::cerr << "Error: Assignment of empty expression to varialbe `" << varName << "'\n";
        return nullptr;
    }
    builder->CreateStore(val, address);
    return nullptr;
}

// if 条件分岐
void CodeGen::visit(IfNode *node){
    llvm::Value* conditionValue = visit(node->condition.get());
    if(!conditionValue) return;
    // ブロック生成
    llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(*context, "then", currentFunction);
    llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(*context, "else"); // else~がなくてもとりあえず生成
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "ifcont");
    // 条件分岐命令を生成
    llvm::BasicBlock* elseDestBlock = node->else_block ? elseBlock : mergeBlock;
    builder->CreateCondBr(conditionValue, thenBlock, elseDestBlock);
    // thenの中身を生成
    builder->SetInsertPoint(thenBlock);
    visit(node->if_block.get());
    builder->CreateBr(mergeBlock);
    // elseの中身を生成
    if (node->else_block) {
        currentFunction->insert(currentFunction->end(), elseBlock);
        builder->SetInsertPoint(elseBlock);
        visit(node->else_block.get());
        builder->CreateBr(mergeBlock);
    }
    // mergeに処理を移す
    currentFunction->insert(currentFunction->end(), mergeBlock);
    builder->SetInsertPoint(mergeBlock);
}

// NumberLiteral 数値リテラル
llvm::Value* CodeGen::visit(NumberLiteralNode *node){
    // int32tyを返す
    return builder->getInt32(node->value);
}

// BinaryOp 二項計算
llvm::Value* CodeGen::visit(BinaryOpNode *node){
    // 右辺と左辺を再帰的にvisit
    llvm::Value* lval = visit(node->left.get());
    llvm::Value* rval = visit(node->right.get());
    if (!lval || !rval) {
        std::cerr << "Error: One of the operands in a binary operation is null.\n";
        return nullptr; 
    }
    // opを確認
    if(node->op == "+"){
        return builder->CreateAdd(lval, rval, "addtmp");
    }else if(node->op == "-"){
        return builder->CreateSub(lval, rval, "subtmp");
    }else if(node->op == "*"){
        return builder->CreateMul(lval, rval, "multmp");
    }else if(node->op == "/"){
        return builder->CreateSDiv(lval, rval, "divtmp");
    }else if(node->op == "=="){
        return builder->CreateICmpEQ(lval, rval, "eqtmp");
    }else if(node->op == "!="){
        return builder->CreateICmpNE(lval, rval, "neqtmp");
    }else if(node->op == "<"){
        return builder->CreateICmpSLT(lval, rval, "lttmp");
    }else if(node->op == ">"){
        return builder->CreateICmpSGT(lval, rval, "gttmp");
    }else if(node->op == "<="){
        return builder->CreateICmpSLE(lval, rval, "letmp");
    }else if(node->op == ">="){
        return builder->CreateICmpSGE(lval, rval, "getmp");
    }

    else{
        std::cerr << "Error: Unknown operator.\n";
        return nullptr;
    }
}

// VariableRef 変数参照
llvm::Value* CodeGen::visit(VariableRefNode *node){
    // シンボルテーブルから変数のアドレスを探す
    llvm::Value* varAddress = namedValues[node->name];
    // 変数がなかったらエラー
    if(!varAddress){
        std::cerr << "Error: Unknown variable name `" << node->name << "'\n";
        return nullptr;
    }
    // load命令を生成
    // 値の型, アドレス, IRのレジスタ名(デバッグ用)
    return builder->CreateLoad(builder->getInt32Ty(), varAddress, node->name.c_str());
}
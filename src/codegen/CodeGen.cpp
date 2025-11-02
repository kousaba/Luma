#include <iostream>
#include <memory>
#include "CodeGen.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
#include "../ast/Definition.h"
#include "llvm/IR/IRBuilder.h"
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
    // 先にすべての関数処理を生成
    for(const auto& stmt : node->statements){
        if(auto funcDef = dynamic_cast<FunctionDefNode*>(stmt.get())){
            visit(funcDef);
        }
    }
    auto funcType = llvm::FunctionType::get(builder->getInt32Ty(), false);
    // 関数名デバッグ用
    std::cout << "Creating function: main" << std::endl;
    auto mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "", module.get());
    mainFunc->setName("main");
    std::cout << "LLVM Function name after setName(): " << mainFunc->getName().str() << std::endl;

    auto entryBlock = llvm::BasicBlock::Create(*context, "entry", mainFunc);
    builder->SetInsertPoint(entryBlock);
    for(const auto& stmt : node->statements){
        if(!dynamic_cast<FunctionDefNode*>(stmt.get())){
            visit(stmt.get());
        }
    }
    builder->CreateRet(builder->getInt32(0));
    // main関数の検証
    if (llvm::verifyFunction(*mainFunc, &llvm::errs())) {
        std::cerr << "LLVM Main Function Verification Failed!\n";
    }
}

// Statement
void CodeGen::visit(StatementNode* node){
    if(auto funcDef = dynamic_cast<FunctionDefNode*>(node)){
        // ProgramNodeで処理するので何もしない
        return;
    }else if(auto varNode = dynamic_cast<VarDeclNode*>(node)){
        visit(varNode);
    }else if(auto assignmentNode = dynamic_cast<AssignmentNode*>(node)){
        visit(assignmentNode);
    }else if(auto ifNode = dynamic_cast<IfNode*>(node)){
        visit(ifNode);
    }else if(auto forNode = dynamic_cast<ForNode*>(node)){
        visit(forNode);
    }else if(auto exprStmtNode = dynamic_cast<ExprStatementNode*>(node)){
        visit(exprStmtNode);
    }
    
    else{
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

// FunctionDefinition
void CodeGen::visit(FunctionDefNode *node){
    llvm::BasicBlock* originalBlock = builder->GetInsertBlock(); // 最後挿入場所を戻すよう
    std::vector<llvm::Type*> argTypes;
    // 今はintしかないのでintを入れる
    for(const auto& type : node->parameters) argTypes.push_back(builder->getInt32Ty());
    // 戻り値の型もとりあえずint
    llvm::Type* returnType = builder->getInt32Ty();
    // 関数登録
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, argTypes, false);
    
    // 関数名デバッグ用
    std::cout << "Creating function: " << node->name << std::endl;
    auto func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "", module.get());
    func->setName(node->name.c_str());
    std::cout << "LLVM Function name after setName(): " << func->getName().str() << std::endl;

    auto entryBlock = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(entryBlock);
    // 新しいスコープのためのシンボルテーブルを用意
    std::map<std::string, llvm::Value*> namedValuesInFunc;
    auto originalNamedValues = namedValues;
    // 関数の引数イテレータとASTの仮引数名リストを同時にループ
    auto param_it = node->parameters.begin();
    for(auto& arg: func->args()){
        // 引数に名前をつける
        std::string paramName = *param_it;
        arg.setName(paramName);
        // メモリ確保・値を代入・シンボルテーブル登録
        auto allocaInst = builder->CreateAlloca(arg.getType(), nullptr, paramName);
        builder->CreateStore(&arg, allocaInst);
        namedValuesInFunc[paramName] = allocaInst;
        param_it++;
    }
    namedValues = namedValuesInFunc;
    visit(node->body.get());

    // return命令がない場合に自動で追加
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateRet(builder->getInt32(0));
    }

    // 関数の検証
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        std::cerr << "LLVM Function " << node->name << " Verification Failed!\n";
    }

    // シンボルテーブルをもとに戻す
    namedValues = originalNamedValues;
    if(originalBlock) builder->SetInsertPoint(originalBlock); // 元の場所に戻す
}

// Expr
llvm::Value* CodeGen::visit(ExprNode *node){
    // dynamic_castでどれにキャストできるか分ける
    if(auto cnode = dynamic_cast<NumberLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<BinaryOpNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VariableRefNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<FunctionCallNode*>(node)) return visit(cnode);
    std::cerr << "Error: ExprNode couldn\'t cast.\n";
    return nullptr;
}

// ExprStmt
void CodeGen::visit(ExprStatementNode* node){
    visit(node->expression.get());
}

// FunctionCall 関数呼び出し
llvm::Value* CodeGen::visit(FunctionCallNode *node){
    // TODO: 組み込み関数以外にも対応させる
    if(node->calleeName == "print") return generatePrintCall(node);
    if(node->calleeName == "input") return generateInputCall(node);
    if(auto func = module->getFunction(node->calleeName)){
        llvm::Function* calleeFunc = module->getFunction(node->calleeName);
        if(!calleeFunc){
            std::cerr << "Error: Call to undefined function '" << node->calleeName << "'\n";
            return nullptr;
        }
        if(calleeFunc->arg_size() != node->args.size()){
            std::cerr << "Error: Incorrect number of arguments passed to function '" << node->calleeName << "'\n";
            return nullptr;
        }
        // ASTの引数リストをvisitして、llvm::Value*のvectorに変換
        std::vector<llvm::Value*> argsVec;
        for(const auto& argExpr : node->args){
            llvm::Value *argValue = visit(argExpr.get());
            if(!argValue){
                std::cerr << "Error: Unknown error happend in function call.\n";
                return nullptr;
            }
            argsVec.push_back(argValue);
        }
        // call命令を生成
        std::string retName = node->calleeName + "_ret";
        return builder->CreateCall(calleeFunc, argsVec, retName);
    }
    return nullptr;
}

// input・print
llvm::Value* CodeGen::generatePrintCall(FunctionCallNode *node){
    // printf関数のプロトタイプを取得
    llvm::Function* printfFunc = module->getFunction("printf");
    if(!printfFunc){
        llvm::Type* charPtrType = llvm::PointerType::get(*context, 0);
        llvm::FunctionType* printfType = llvm::FunctionType::get(
            builder->getInt32Ty(),
            charPtrType,
            true
        );
        printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module.get());
    }
    // 引数を準備
    std::vector<llvm::Value*> args;
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%d\n");
    args.push_back(formatStr);
    for(const auto& argExpr : node->args){
        args.push_back(visit(argExpr.get()));
    }
    return builder->CreateCall(printfFunc, args, "printfCall");
}
llvm::Value* CodeGen::generateInputCall(FunctionCallNode *node){
    // scanf関数のプロトタイプを取得
    llvm::Function* scanfFunc = module->getFunction("scanf");
    if(!scanfFunc){
        llvm::Type* charPtrType = llvm::PointerType::get(*context, 0);
        llvm::FunctionType* scanfType = llvm::FunctionType::get(builder->getInt32Ty(), charPtrType, true);
        scanfFunc = llvm::Function::Create(scanfType, llvm::Function::ExternalLinkage, "scanf", module.get());
    }
    // 引数を準備
    std::vector<llvm::Value*> args;
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%d");
    args.push_back(formatStr);
    if(node->args.size() != 1){
        std::cerr << "Error: too many arguments to input function.\n";
        return nullptr;
    }
    auto varRef = std::dynamic_pointer_cast<VariableRefNode>(node->args[0]);
    if(!varRef){
        std::cerr << "Error: Unknown argument to input function.\n";
        return nullptr;
    }
    llvm::Value* varAddress = namedValues[varRef->name];
    if(!varAddress){
        std::cerr << "Error: unknown variable to input function.\n";
        return nullptr;
    }
    args.push_back(varAddress);
    return builder->CreateCall(scanfFunc, args, "scanfCall");
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
    if (!conditionValue) {
        std::cerr << "Error: The condition expression of if is an unknown expression.\n";
        return;
    }
    // ブロック生成
    llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(*context, "then", currentFunction);
    llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(*context, "else"); // else~がなくてもとりあえず生成
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "ifcond");
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

// for
void CodeGen::visit(ForNode *node){
    // ブロック生成
    llvm::Function* currentFunction = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* loopHeaderBlock = llvm::BasicBlock::Create(*context, "loopHeader", currentFunction);
    llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(*context, "loopBody", currentFunction);
    llvm::BasicBlock* afterLoopBlock = llvm::BasicBlock::Create(*context, "afterLoop", currentFunction);
    builder->CreateBr(loopHeaderBlock);
    builder->SetInsertPoint(loopHeaderBlock);
    llvm::Value* conditionValue = visit(node->condition.get());
    if (!conditionValue) {
        std::cerr << "Error: The condition expression of for is an unknown expression.\n";
        return;
    }
    builder->CreateCondBr(conditionValue, loopBodyBlock, afterLoopBlock);
    builder->SetInsertPoint(loopBodyBlock);
    visit(node->block.get());
    builder->CreateBr(loopHeaderBlock);
    builder->SetInsertPoint(afterLoopBlock);
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
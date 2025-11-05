#include <cassert>
#include <iostream>
#include <llvm-18/llvm/IR/Constants.h>
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
#include "common/Global.h"

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

std::unique_ptr<llvm::Module> CodeGen::releaseModule(){
    return std::move(module);
}

llvm::Type* CodeGen::translateType(std::string name){
    if(name == "int") return builder->getInt64Ty();
    if(name == "int32" || name == "i32") return builder->getInt32Ty();
    if(name == "char") return builder->getInt8Ty();
    if(name == "float") return builder->getDoubleTy();
    if(name == "float32") return builder->getFloatTy();
    errorHandler.errorReg("Unknown Type.", 0);
    return nullptr;
}

// Program
void CodeGen::visit(ProgramNode* node){
    // main関数を定義
    llvm::FunctionType* funcType = llvm::FunctionType::get(builder->getInt32Ty(), false);
    llvm::Function* mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module.get());
    // エントリーブロック作成
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*context, "entry", mainFunc);
    builder->SetInsertPoint(entryBlock);
    // main関数スコープのためのシンボルテーブル
    auto originalNamedValues = namedValues;
    namedValues.clear();
    // 各文をvisit
    for(auto& stmt : node->statements) visit(stmt.get());
    // 最後にreturn 0;を追加
    builder->CreateRet(builder->getInt32(0));
    // もとに戻す
    namedValues = originalNamedValues;
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
        errorHandler.errorReg("Unknown Statement visited.", 0);
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
    errorHandler.errorReg("Creating function: " + node->name, 2);
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
        errorHandler.errorReg("LLVM Function " + node-> name + " Vertification Failed!", 0);
    }

    // シンボルテーブルをもとに戻す
    namedValues = originalNamedValues;
    if(originalBlock) builder->SetInsertPoint(originalBlock); // 元の場所に戻す
}

// Expr
llvm::Value* CodeGen::visit(ExprNode *node){
    // dynamic_castでどれにキャストできるか分ける
    if(auto cnode = dynamic_cast<NumberLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<DecimalLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<BinaryOpNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VariableRefNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<FunctionCallNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<CastNode*>(node)) return visit(cnode);
    std::cerr << "Error: ExprNode couldn\'t cast.\n";
    errorHandler.errorReg("ExprNode couldn't cast.\n\t This is compilers error.", 0);
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
            errorHandler.errorReg("Call to undefined function '" + node->calleeName, 0);
            return nullptr;
        }
        if(calleeFunc->arg_size() != node->args.size()){
            errorHandler.errorReg("Incorrect number of arguments passed to function '" + node->calleeName , 0);
            return nullptr;
        }
        // ASTの引数リストをvisitして、llvm::Value*のvectorに変換
        std::vector<llvm::Value*> argsVec;
        for(const auto& argExpr : node->args){
            llvm::Value *argValue = visit(argExpr.get());
            errorHandler.conditionErrorReg(!argValue, "Unknown error happend in function call.\n", 0);
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
    std::string formatStr = "";

    for(const auto& argExpr : node->args){
        llvm::Value* argValue = visit(argExpr.get());
        if(!argValue){
            errorHandler.errorReg("argValue is null in print call.", 0);
            return nullptr;
        }
        // それぞれの型ごとにフォーマット文字列を変更
        if(argValue->getType()->isIntegerTy(32)) formatStr += "%d";
        else if(argValue->getType()->isIntegerTy(64)) formatStr += "%lld";
        else if(argValue->getType()->isDoubleTy() || argValue->getType()->isFloatTy()) formatStr += "%f";
        // TODO: 文字列等に対応
        else{
            errorHandler.errorReg("Unsupported type for print",0);
            return nullptr;
        }
        args.push_back(argValue); // 引数に値を追加
    }
    formatStr += "\n";
    llvm::Value* formatStrVal = builder->CreateGlobalStringPtr(formatStr);
    args.insert(args.begin(), formatStrVal);
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
    std::string formatStr = "";
    if(node->args.size() != 1){
        errorHandler.errorReg("too many arguments to input function.", 0);
        return nullptr;
    }
    auto varRef = std::dynamic_pointer_cast<VariableRefNode>(node->args[0]);
    if(!varRef){
        errorHandler.errorReg("Unknown argument to input function.", 0);
        return nullptr;
    }
    llvm::Value* varAddress = namedValues[varRef->name];
    if(!varAddress){
        errorHandler.errorReg("Unknown variable to input function.", 0);
        return nullptr;
    }
    auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(varAddress);
    llvm::Type* varType = allocaInst->getAllocatedType();
    if(varType->isIntegerTy(32)) formatStr += "%d";
    else if(varType->isIntegerTy(64)) formatStr += "%lld";
    else if(varType->isDoubleTy() || varType->isFloatTy()) formatStr += "%f";
    // TODO: 文字列等に対応
    else{
        errorHandler.errorReg("Unsupported type for input",0);
        return nullptr;
    }
    llvm::Value* formatStrVal = builder->CreateGlobalStringPtr(formatStr);
    args.push_back(formatStrVal);
    args.push_back(varAddress);
    return builder->CreateCall(scanfFunc, args, "scanfCall");
}

// varDecl 変数宣言
llvm::Value* CodeGen::visit(VarDeclNode* node){
    std::string varName = node->varName;
    llvm::Type* varType = nullptr;
    if(node->type){
        varType = translateType(node->type->getTypeName());
    }else if(node->initializer){
        // TODO: 複数型対応
        varType = builder->getInt64Ty();
    }else{
        // 型注釈も初期化式もないとき
        errorHandler.errorReg("The variable declaration has neither a type annotation nor an initialization expression.", 1); // 警告を出す
        varType = builder->getInt64Ty();
    }
    if(!varType){
        // translateTypeがエラーメッセージを出しているはず
        return nullptr;
    }
    auto allocaInst = builder->CreateAlloca(varType, nullptr, varName);
    // シンボルテーブルに登録
    namedValues[node->varName] = allocaInst;
    if(node->initializer){
        llvm::Value* initVal = visit(node->initializer.get());
        if(initVal)
        // TODO: 型チェックとキャスト(int -> i32)
        builder->CreateStore(initVal, allocaInst);
    }
    return 0;
}

// assignment 代入
llvm::Value* CodeGen::visit(AssignmentNode *node){
    std::string varName = node->varName;
    auto it = namedValues.find(varName);
    if(it == namedValues.end()){
        errorHandler.errorReg("Assignment to undeclared variable '" + varName + "'",0);
        return nullptr;
    }
    auto address = namedValues[varName];
    llvm::Value* val = visit(node->value.get());
    if(!val){
        errorHandler.errorReg("Assignment of empty expression to variable '" + varName + "'", 0);
        return nullptr;
    }
    builder->CreateStore(val, address);
    return nullptr;
}

// if 条件分岐
void CodeGen::visit(IfNode *node){
    llvm::Value* conditionValue = visit(node->condition.get());
    errorHandler.conditionErrorReg(!conditionValue, "The condition expression of if is an unknown expression.", 1);
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
    errorHandler.conditionErrorReg(!conditionValue, "The condition expression of for is an unknown expression.", 1);
    builder->CreateCondBr(conditionValue, loopBodyBlock, afterLoopBlock);
    builder->SetInsertPoint(loopBodyBlock);
    visit(node->block.get());
    builder->CreateBr(loopHeaderBlock);
    builder->SetInsertPoint(afterLoopBlock);
}

// NumberLiteral 数値リテラル
llvm::Value* CodeGen::visit(NumberLiteralNode *node){
    // int32tyを返す
    return builder->getInt64(node->value);
}

// DecimalLiteral 小数リテラル
llvm::Value* CodeGen::visit(DecimalLiteralNode *node){
    // doubleTyを返す
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), node->value);
}

// BinaryOp 二項計算
llvm::Value* CodeGen::visit(BinaryOpNode *node){
    // 右辺と左辺を再帰的にvisit
    llvm::Value* lval = visit(node->left.get());
    llvm::Value* rval = visit(node->right.get());
    if (!lval || !rval) {
        errorHandler.errorReg("One of the operands in a binary operation is null.", 0);
        return nullptr; 
    }
    if(lval->getType() != rval->getType()){
        errorHandler.errorReg("Currently type casting is not supported.", 0);
        return nullptr;
    }
    // opを確認
    if(lval->getType()->isIntegerTy(32) || lval->getType()->isIntegerTy(64)){
        if(node->op == "+") return builder->CreateAdd(lval, rval, "addtmp");
        else if(node->op == "-") return builder->CreateSub(lval, rval, "subtmp");
        else if(node->op == "*") return builder->CreateMul(lval, rval, "multmp");
        else if(node->op == "/") return builder->CreateSDiv(lval, rval, "divtmp");
        else if(node->op == "==") return builder->CreateICmpEQ(lval, rval, "eqtmp");
        else if(node->op == "!=") return builder->CreateICmpNE(lval, rval, "neqtmp");
        else if(node->op == "<") return builder->CreateICmpSLT(lval, rval, "lttmp");
        else if(node->op == ">") return builder->CreateICmpSGT(lval, rval, "gttmp");
        else if(node->op == "<=") return builder->CreateICmpSLE(lval, rval, "letmp");
        else if(node->op == ">=") return builder->CreateICmpSGE(lval, rval, "getmp");
        else{
            errorHandler.errorReg("Unknown operator in integer.", 0);
            return nullptr;
        }
    }else if(lval->getType()->isDoubleTy()){
        if(node->op == "+") return builder->CreateFAdd(lval, rval, "faddtmp");
        else if(node->op == "-") return builder->CreateFSub(lval, rval, "fsubtmp");
        else if(node->op == "*") return builder->CreateFMul(lval, rval, "fmultmp");
        else if(node->op == "/") return builder->CreateFDiv(lval, rval, "fdivtmp");
        else if(node->op == "==") return builder->CreateFCmpOEQ(lval, rval, "feqtmp");
        else if(node->op == "!=") return builder->CreateFCmpONE(lval, rval, "fneqtmp");
        else if(node->op == "<") return builder->CreateFCmpOLT(lval, rval, "flttmp");
        else if(node->op == ">") return builder->CreateFCmpOGT(lval, rval, "fgttmp");
        else if(node->op == "<=") return builder->CreateFCmpOLE(lval, rval, "fletmp");
        else if(node->op == ">=") return builder->CreateFCmpOGE(lval, rval, "fgetmp");
        else{
            errorHandler.errorReg("Unknown operator in decimal.",0);
            return nullptr;
        }
    }else{
        errorHandler.errorReg("Unknown binary operator.", 0);
        return nullptr;
    }
}

// Cast キャスト
llvm::Value* CodeGen::visit(CastNode *node){
    llvm::Value* originalValue = visit(node->expression.get());
    if(!originalValue){
        errorHandler.errorReg("expression to cast to is empty.", 0);
        return nullptr;
    }
    llvm::Type* originalLlvmType = originalValue->getType(); // キャスト元の型
    std::shared_ptr<TypeNode> targetTypeNode = node->targetType;
    if(!targetTypeNode){
        errorHandler.errorReg("Target type for cast is null.", 0);
        return nullptr;
    }
    llvm::Type* targetLlvmType = translateType(targetTypeNode->getTypeName());
    if(!targetLlvmType){
        // translateTypeないでエラーが登録されている
        return nullptr;
    }
    // キャスト処理
    if(originalLlvmType->isIntegerTy() && targetLlvmType->isIntegerTy()) return builder->CreateIntCast(originalValue, targetLlvmType, true, "intcasttmp");
    else if(originalLlvmType->isFloatingPointTy() && targetLlvmType->isFloatingPointTy()) return builder->CreateFPCast(originalValue, targetLlvmType, "fpcasttmp");
    else if(originalLlvmType->isIntegerTy() && targetLlvmType->isFloatingPointTy()) return builder->CreateSIToFP(originalValue, targetLlvmType, "sitofptmp");
    else if(originalLlvmType->isFloatingPointTy() && targetLlvmType->isIntegerTy()) return builder->CreateFPToSI(originalValue, targetLlvmType, "fptositmp");
    else{
        errorHandler.errorReg("Unsupported cast operation.", 0);
        return nullptr;
    }
}

// VariableRef 変数参照
llvm::Value* CodeGen::visit(VariableRefNode *node){
    // シンボルテーブルから変数のアドレスを探す
    auto it = namedValues.find(node->name);
    if (it == namedValues.end()) {
        errorHandler.errorReg("Unknown variable name '" + node->name + "'", 0);
        return nullptr;
    }
    llvm::Value* varAddress = it->second;
    // 変数がなかったらエラー
    if(!varAddress){
        errorHandler.errorReg("Unknown variable name '" + node->name + "'", 0);
        return nullptr;
    }
    // load命令を生成
    // 値の型, アドレス, IRのレジスタ名(デバッグ用)
    auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(varAddress);
    return builder->CreateLoad(allocaInst->getAllocatedType(), varAddress, node->name.c_str());
}
#pragma once
#include <map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "../ast/AstNode.h"
#include "../ast/Statement.h"
#include "../ast/Definition.h"

class CodeGen{
private:
    // LLVMの全体的な状態を管理するコンテナ
    std::unique_ptr<llvm::LLVMContext> context;
    // 一つのソースファイルに対応するトップレベルコンテナ
    // 関数やグローバル変数など
    std::unique_ptr<llvm::Module> module;
    // IRの命令を生成するためのヘルパークラス
    std::unique_ptr<llvm::IRBuilder<>> builder;
    // シンボルテーブル(関数名とメモリアドレス(llvm::Value*))を対応付けるmap
    std::map<std::string, llvm::Value*> namedValues;
public:
    CodeGen();
    // ASTのルートノードを受け取り、コード生成を開始するメインメソッド
    void generate(ProgramNode *root);
    // 生成したModuleを外部に渡すメソッド
    llvm::Module* getModule();
    std::unique_ptr<llvm::Module> releaseModule();
private:
    // ASTノードごとのvisitメソッド
    void visit(ProgramNode *node);
    void visit(StatementNode *node);
    void visit(BlockNode *node);
    void visit(FunctionDefNode *node);
    llvm::Value* visit(VarDeclNode *node);
    llvm::Value* visit(AssignmentNode *node);
    void visit(IfNode *node);
    void visit(ForNode *node);
    llvm::Value* visit(ExprNode *node);
    void visit(ExprStatementNode* node);
    llvm::Value* visit(FunctionCallNode *node);
    llvm::Value* visit(NumberLiteralNode *node);
    llvm::Value* visit(BinaryOpNode *node);
    llvm::Value* visit(VariableRefNode *node);

    // print・input用
    llvm::Value* generatePrintCall(FunctionCallNode* node);
    llvm::Value* generateInputCall(FunctionCallNode* node);
};
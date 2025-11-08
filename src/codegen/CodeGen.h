#pragma once
#include <map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "../ast/AstNode.h"
#include "../ast/Statement.h"
#include "../ast/Definition.h"

// セマンティック解析
#include "semantic/SemanticAnalysis.h"

class CodeGen{
private:
    // LLVMの全体的な状態を管理するコンテナ
    std::unique_ptr<llvm::LLVMContext> context;
    // 一つのソースファイルに対応するトップレベルコンテナ
    // 関数やグローバル変数など
    std::unique_ptr<llvm::Module> module;
    // IRの命令を生成するためのヘルパークラス
    std::unique_ptr<llvm::IRBuilder<>> builder;
    // Lumaの型からLLVMの型に変換する
    llvm::Type* translateType(std::string typeName);
    // SemanticAnalysisへの参照
    SemanticAnalysis& semanticAnalysis;
public:
    CodeGen(SemanticAnalysis& sema);
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
    void visit(VarDeclNode *node);
    void visit(AssignmentNode *node);
    void visit(IfNode *node);
    void visit(ForNode *node);
    llvm::Value* visit(ExprNode *node);
    llvm::Value* visit(CastNode *node);
    void visit(ExprStatementNode* node);
    llvm::Value* visit(FunctionCallNode *node);
    llvm::Value* visit(NumberLiteralNode *node);
    llvm::Value* visit(DecimalLiteralNode *node);
    llvm::Value* visit(BinaryOpNode *node);
    llvm::Value* visit(VariableRefNode *node);

    // print・input用
    llvm::Value* generatePrintCall(FunctionCallNode* node);
    llvm::Value* generateInputCall(FunctionCallNode* node);
};
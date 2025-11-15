#pragma once

#include <memory>
#include <string>
#include <map>

// MIRヘッダー
#include "mir/MIRModule.h"
#include "mir/MIRFunction.h"
#include "mir/MIRBasicBlock.h"
#include "mir/MIRInstruction.h"
#include "mir/MIRTerminator.h"
#include "mir/MIRType.h"
#include "mir/MIRValue.h"

// ASTヘッダー
#include "../ast/AstNode.h"
#include "../ast/Statement.h"
#include "../ast/Expression.h"
#include "../ast/Definition.h"

// セマンティック解析ヘッダー
#include "../semantic/SemanticAnalysis.h"
#include "../types/Type.h" // TypeNode用

class MIRGen {
public:
    MIRGen(SemanticAnalysis& sema);

    // MIR生成のトップレベル関数
    std::unique_ptr<MIRModule> generate(std::shared_ptr<ProgramNode> root);

private:
    // 状態管理
    SemanticAnalysis& semanticAnalysis; // セマンティック解析への参照
    std::unique_ptr<MIRModule> module; // 生成中のMIRモジュール
    std::shared_ptr<MIRFunction> currentFunction = nullptr; // 現在処理中の関数
    std::shared_ptr<MIRBasicBlock> currentBlock = nullptr; // 現在命令を追加中の基本ブロック
    
    // ASTシンボルとMIRの値（メモリアドレス）のマッピング
    std::map<std::shared_ptr<Symbol>, std::shared_ptr<MIRValue>> symbolValueMap;
    
    // 一時レジスタのカウンタ
    int tempCounter = 0;
    std::string newRegisterName(); // 新しい一時レジスタ名を生成

    // ヘルパー関数
    void setCurrentBlock(std::shared_ptr<MIRBasicBlock> block); // 現在の基本ブロックを設定
    std::shared_ptr<MIRBasicBlock> createBasicBlock(const std::string& name); // 新しい基本ブロックを作成

    // ASTノードごとのvisitメソッド
    void visit(ProgramNode* node);
    void visit(StatementNode* node);
    void visit(BlockNode* node);
    void visit(FunctionDefNode* node);
    void visit(VarDeclNode* node);
    void visit(ArrayDeclNode *node);
    void visit(AssignmentNode* node);
    void visit(IfNode* node);
    void visit(ForNode* node);
    void visit(ReturnNode* node);
    void visit(ExprStatementNode* node);

    std::shared_ptr<MIRValue> visit(ExprNode* node);
    std::shared_ptr<MIRValue> visit(NumberLiteralNode* node);
    std::shared_ptr<MIRValue> visit(DecimalLiteralNode* node);
    std::shared_ptr<MIRValue> visit(VariableRefNode* node);
    std::shared_ptr<MIRValue> visit(ArrayRefNode *node);
    std::shared_ptr<MIRValue> visit(BinaryOpNode* node);
    std::shared_ptr<MIRValue> visit(FunctionCallNode* node);
    std::shared_ptr<MIRValue> visit(CastNode* node);
};
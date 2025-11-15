#pragma once
#include "ast/Definition.h"
#include "types/Type.h"
#include <llvm/IR/Value.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <memory>
#include "Symbol.h"

// Forward declarations for AST nodes
class ProgramNode;
class BlockNode;
class IfNode;
class ForNode;
class ReturnNode;
class VarDeclNode;
class AssignmentNode;
class ExprStatementNode;
class StatementNode;
class BinaryOpNode;
class FunctionCallNode;
class NumberLiteralNode;
class DecimalLiteralNode;
class VariableRefNode;
class CastNode;
class ExprNode;


class SemanticAnalysis{
private:
    std::shared_ptr<Scope> currentScope;
    void enterScope();
    void leaveScope();
    // 基本型
    std::unordered_map<std::string, std::shared_ptr<TypeNode>> typePtr;
    bool is_type(std::string typeName);
    TypeNode* currentFunctionReturnType = nullptr;
public:
    // コンストラクタ
    SemanticAnalysis();
    // main用
    // エラーがあるか
    bool hasErrors();
    std::shared_ptr<TypeNode> getType(const std::string& name) const; // ゲッターを追加
    void analyze(std::shared_ptr<ProgramNode> root);
private:
    // visit処理
    std::shared_ptr<TypeNode> visit(BinaryOpNode *node);
    std::shared_ptr<TypeNode> visit(FunctionCallNode *node);
    void visit(AssignmentNode *node);
    std::shared_ptr<TypeNode> visit(NumberLiteralNode *node);
    std::shared_ptr<TypeNode> visit(DecimalLiteralNode *node);
    std::shared_ptr<TypeNode> visit(VariableRefNode *node);
    std::shared_ptr<TypeNode> visit(ArrayRefNode *node);
    std::shared_ptr<TypeNode> visit(CastNode *node);
    void visit(VarDeclNode *node);
    void visit(ArrayDeclNode *node);
    std::shared_ptr<TypeNode> visit(ExprNode *node); // 振り分け用
    // 式以外
    void visit(FunctionDefNode *node);
    void visit(ProgramNode *node); // 各要素をvisit
    void visit(BlockNode *node); // enterScope() -> visit -> leaveScope()
    void visit(IfNode *node); // conditionをvisit -> boolか確認 -> thenとelseをvisit(blockなのでvisit(if_block)のみでいい)
    void visit(ForNode *node); // Ifと同じ
    void visit(ReturnNode *node); // fnの戻り値と型が同じか調べる
    void visit(ExprStatementNode *node); // 中身のexprを呼び出すだけ
    void visit(StatementNode *node); // 振り分け用
private:
    std::shared_ptr<Scope> globalScope;
    std::shared_ptr<FuncSymbol> currentFunction;
};
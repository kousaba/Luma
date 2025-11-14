#pragma once
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
    std::vector<std::map<std::string, std::shared_ptr<Symbol>>> symbolTable;
    void enterScope();
    void leaveScope();
    bool addSymbol(std::shared_ptr<Symbol> symbol);
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
    std::shared_ptr<Symbol> lookupSymbol(const std::string& name);
    std::shared_ptr<TypeNode> getType(const std::string& name) const; // ゲッターを追加
    // visit処理
    TypeNode* visit(BinaryOpNode *node);
    TypeNode* visit(FunctionCallNode *node);
    void visit(AssignmentNode *node);
    TypeNode* visit(NumberLiteralNode *node);
    TypeNode* visit(DecimalLiteralNode *node);
    TypeNode* visit(VariableRefNode *node);
    TypeNode* visit(CastNode *node);
    void visit(VarDeclNode *node);
    TypeNode* visit(ExprNode *node); // 振り分け用
    // 式以外
    void visit(ProgramNode *node); // 各要素をvisit
    void visit(BlockNode *node); // enterScope() -> visit -> leaveScope()
    void visit(IfNode *node); // conditionをvisit -> boolか確認 -> thenとelseをvisit(blockなのでvisit(if_block)のみでいい)
    void visit(ForNode *node); // Ifと同じ
    void visit(ReturnNode *node); // fnの戻り値と型が同じか調べる
    void visit(ExprStatementNode *node); // 中身のexprを呼び出すだけ
    void visit(StatementNode *node); // 振り分け用
};
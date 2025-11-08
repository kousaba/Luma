#pragma once
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "types/Type.h"
#include <llvm/IR/Value.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>

enum class SymbolKind{
    VAR,
    FUNC,
    STRUCT
};

struct Symbol{
    SymbolKind kind;
    std::string name;
    TypeNode* type;
    llvm::Value* llvmValue = nullptr;
    std::vector<TypeNode*> argTypes;
    
    Symbol(const std::string& name, TypeNode* type) : kind(SymbolKind::VAR), name(name), type(type) {} // 変数用
    Symbol(const std::string& name, TypeNode* returnType, const std::vector<TypeNode*>& args) : kind(SymbolKind::FUNC), name(name), type(returnType), argTypes(args){}// 関数用
    Symbol() = delete;
};

class SemanticAnalysis{
private:
    std::vector<std::map<std::string, std::unique_ptr<Symbol>>> symbolTable;
    void enterScope();
    void leaveScope();
    bool addSymbol(std::unique_ptr<Symbol> symbol);
    // 基本型
    std::unordered_map<std::string, std::shared_ptr<TypeNode>> typePtr;
    bool is_type(std::string typeName);
public: // ★ここから public にする
    // コンストラクタ
    SemanticAnalysis();
    // main用
    // エラーがあるか
    bool hasErrors();
    Symbol* lookupSymbol(const std::string& name); // ★ここへ移動
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
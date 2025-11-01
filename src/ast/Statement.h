#pragma once
#include "AstNode.h"
#include "../ast/Expression.h"
#include <memory>
#include <string>
#include <vector>

// 文の基底クラス
class StatementNode : public AstNode {};


// 変数宣言文を表すノード
class VarDeclNode : public StatementNode{
public:
    std::string varName;
    // 初期化式を保持するポインタ(式がない場合はnullptr)
    std::shared_ptr<ExprNode> initializer;
    VarDeclNode(const std::string& name, std::shared_ptr<ExprNode> init = nullptr) : varName(name), initializer(init){}
};

// プログラム全体を表すノード（文のリストを持つ）
class ProgramNode : public AstNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;
};
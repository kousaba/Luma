#pragma once
#include "AstNode.h"
#include <memory>
#include <string>
#include <vector>

// 文の基底クラス
class StatementNode : public AstNode {};


// 変数宣言文を表すノード
class VarDeclNode : public StatementNode{
public:
    std::string varName;
    VarDeclNode(const std::string& name) : varName(name){}
};

// プログラム全体を表すノード（文のリストを持つ）
class ProgramNode : public AstNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;
};
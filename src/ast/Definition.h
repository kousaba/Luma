#pragma once
#include "AstNode.h"
#include "Statement.h"
#include <memory>
#include <string>
#include <vector>

// 関数・構造体などの基底クラス
class DefNode : public AstNode{};

// 関数宣言を表すノード
class FunctionDefNode : public StatementNode{
public: 
    std::string name;
    std::vector<std::string> parameters;
    std::shared_ptr<BlockNode> body;
    FunctionDefNode(const std::string& n, const std::vector<std::string>& params, std::shared_ptr<BlockNode> b) : name(n), parameters(params), body(b) {}
};
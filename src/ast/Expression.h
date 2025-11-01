#pragma once
#include "AstNode.h"
#include <memory>
#include <string>

// 式の基底クラス
class ExprNode : public AstNode{};

// 数値リテラルを表すノード
class NumberLiteralNode : public ExprNode{
public:
    int value;
    NumberLiteralNode(int val) : value(val){}
};

// 二項演算子(例: a + b)を表すノード
class BinaryOpNode : public ExprNode{
public:
    std::string op;
    std::shared_ptr<ExprNode> left;
    std::shared_ptr<ExprNode> right;
    BinaryOpNode(const std::string& op, std::shared_ptr<ExprNode> lhs, std::shared_ptr<ExprNode> rhs) : op(op), left(lhs), right(rhs) {}
};

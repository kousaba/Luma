#pragma once
#include "AstNode.h"
#include <memory>
#include <string>
#include <vector>
#include "../../src/types/Type.h" // Include Type.h

// 式の基底クラス
class ExprNode : public AstNode{};

// 数値リテラルを表すノード
class NumberLiteralNode : public ExprNode{
public:
    int value;
    std::shared_ptr<TypeNode> type;
    NumberLiteralNode(int val) : value(val){}
};
// 小数リテラルを表すノード
class DecimalLiteralNode : public ExprNode{
public:
    double value;
    std::shared_ptr<TypeNode> type;
    DecimalLiteralNode(double val) : value(val){}
};

// 変数参照を表すノード
class VariableRefNode : public ExprNode{
public:
    std::string name;
    std::shared_ptr<TypeNode> type;
    VariableRefNode(const std::string& varName) : name(varName) {}
};

// 二項演算子(例: a + b)を表すノード
class BinaryOpNode : public ExprNode{
public:
    std::string op;
    std::shared_ptr<ExprNode> left;
    std::shared_ptr<ExprNode> right;
    std::shared_ptr<TypeNode> type;
    BinaryOpNode(const std::string& op, std::shared_ptr<ExprNode> lhs, std::shared_ptr<ExprNode> rhs) : op(op), left(lhs), right(rhs) {}
};

// 関数呼び出しを表すノード
class FunctionCallNode : public ExprNode{
public:
    std::string calleeName; // 関数名
    std::vector<std::shared_ptr<ExprNode>> args;
    FunctionCallNode(const std::string& name, std::vector<std::shared_ptr<ExprNode>> arguments) : calleeName(name), args(arguments) {}
};

// キャストを表すノード
class CastNode : public ExprNode{
public:
    std::shared_ptr<ExprNode> expression;
    std::shared_ptr<TypeNode> targetType;
    CastNode(std::shared_ptr<ExprNode> expr, std::shared_ptr<TypeNode> type) : expression(expr), targetType(type){}
};
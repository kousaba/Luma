#pragma once
#include "AstNode.h"
#include "Expression.h"
#include <memory>
#include <string>
#include <vector>

// 文の基底クラス
class StatementNode : public AstNode {};

// プログラム全体を表すノード（文のリストを持つ）
class ProgramNode : public AstNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;
};

// ブロックを表すノード
class BlockNode : public AstNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;
};


// 変数宣言文を表すノード
class VarDeclNode : public StatementNode{
public:
    std::string varName;
    // 初期化式を保持するポインタ(式がない場合はnullptr)
    std::shared_ptr<ExprNode> initializer;
    VarDeclNode(const std::string& name, std::shared_ptr<ExprNode> init = nullptr) : varName(name), initializer(init){}
};
// 変数代入文を表すノード
class AssignmentNode : public StatementNode{
public:
    std::string varName;
    std::shared_ptr<ExprNode> value;
    AssignmentNode(const std::string& name, std::shared_ptr<ExprNode> val) : varName(name), value(val){}
};
// if文を表すノード
class IfNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> condition;
    std::shared_ptr<BlockNode> if_block;
    std::shared_ptr<BlockNode> else_block;
    IfNode(std::shared_ptr<ExprNode> cond, std::shared_ptr<BlockNode> ifblock, std::shared_ptr<BlockNode> elseblock = nullptr) : condition(cond), if_block(ifblock), else_block(elseblock){}
};
// for文を表すノード
class ForNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> condition;
    std::shared_ptr<BlockNode> block;
    ForNode(std::shared_ptr<ExprNode> cond, std::shared_ptr<BlockNode> bnode) : condition(cond), block(bnode){}
};
// 式文を表すノード(print(123); や a = 5;など)
class ExprStatementNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> expression;
    ExprStatementNode(std::shared_ptr<ExprNode> expr) : expression(expr) {}
};
// Return文を表すノード
class ReturnNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> returnValue;
    ReturnNode(std::shared_ptr<ExprNode> val) : returnValue(val) {}
};
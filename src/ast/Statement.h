#pragma once
#include "AstNode.h"
#include "Expression.h"
#include "types/Type.h"
#include <memory>
#include <string>
#include <vector>

// 文の基底クラス
class StatementNode : public AstNode {};

// プログラム全体を表すノード（文のリストを持つ）
class ProgramNode : public AstNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements; // 文のリスト
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ProgramNode {" << std::endl;
        for (const auto& stmt : statements) {
            stmt->dump(indent + 1);
        }
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};

// ブロックを表すノード
class BlockNode : public AstNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements; // 文のリスト
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "BlockNode {" << std::endl;
        for (const auto& stmt : statements) {
            stmt->dump(indent + 1);
        }
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};


// 変数宣言文を表すノード
class VarDeclNode : public StatementNode{
public:
    std::string varName; // 変数名
    // 初期化式を保持するポインタ(式がない場合はnullptr)
    std::shared_ptr<ExprNode> initializer;
    std::shared_ptr<TypeNode> type; // 変数の型
    VarDeclNode(const std::string& name, std::shared_ptr<TypeNode> t, std::shared_ptr<ExprNode> init) : varName(name), type(t),initializer(init){}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDeclNode (Name: " << varName << ", Type: " << (type ? type->getTypeName() : "unknown") << ") {" << std::endl;
        if (initializer) {
            printIndent(indent + 1);
            std::cout << "Initializer: " << std::endl;
            initializer->dump(indent + 2);
        }
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
// 変数代入文を表すノード
class AssignmentNode : public StatementNode{
public:
    std::string varName; // 代入先の変数名
    std::shared_ptr<ExprNode> value; // 代入する値の式
    AssignmentNode(const std::string& name, std::shared_ptr<ExprNode> val) : varName(name), value(val){}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "AssignmentNode (Var: " << varName << ") {" << std::endl;
        printIndent(indent + 1);
        std::cout << "Value: " << std::endl;
        value->dump(indent + 2);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
// if文を表すノード
class IfNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> condition; // 条件式
    std::shared_ptr<BlockNode> if_block; // ifブロック
    std::shared_ptr<BlockNode> else_block; // elseブロック (オプション)
    IfNode(std::shared_ptr<ExprNode> cond, std::shared_ptr<BlockNode> ifblock, std::shared_ptr<BlockNode> elseblock = nullptr) : condition(cond), if_block(ifblock), else_block(elseblock){}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "IfNode {" << std::endl;
        printIndent(indent + 1);
        std::cout << "Condition: " << std::endl;
        condition->dump(indent + 2);
        printIndent(indent + 1);
        std::cout << "If Block: " << std::endl;
        if_block->dump(indent + 2);
        if (else_block) {
            printIndent(indent + 1);
            std::cout << "Else Block: " << std::endl;
            else_block->dump(indent + 2);
        }
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
// for文を表すノード
class ForNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> condition; // 条件式
    std::shared_ptr<BlockNode> block; // forブロック
    ForNode(std::shared_ptr<ExprNode> cond, std::shared_ptr<BlockNode> bnode) : condition(cond), block(bnode){}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ForNode {" << std::endl;
        printIndent(indent + 1);
        std::cout << "Condition: " << std::endl;
        condition->dump(indent + 2);
        printIndent(indent + 1);
        std::cout << "Block: " << std::endl;
        block->dump(indent + 2);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
// 式文を表すノード(print(123); や a = 5;など)
class ExprStatementNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> expression; // 式
    ExprStatementNode(std::shared_ptr<ExprNode> expr) : expression(expr) {}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ExprStatementNode {" << std::endl;
        expression->dump(indent + 1);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
// Return文を表すノード
class ReturnNode : public StatementNode{
public:
    std::shared_ptr<ExprNode> returnValue; // 戻り値の式 (オプション)
    ReturnNode(std::shared_ptr<ExprNode> val) : returnValue(val) {}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ReturnNode {" << std::endl;
        if (returnValue) {
            returnValue->dump(indent + 1);
        } else {
            printIndent(indent + 1);
            std::cout << "void" << std::endl;
        }
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
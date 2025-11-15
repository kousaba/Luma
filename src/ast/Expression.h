#pragma once
#include "AstNode.h"
#include <memory>
#include <string>
#include <vector>
#include "../../src/types/Type.h" // Type.h をインクルード
#include "semantic/Symbol.h"

// 式の基底クラス
class ExprNode : public AstNode{
public:
    std::shared_ptr<TypeNode> type = nullptr; // 式の型情報
    // ExprNodeのダンプは純粋仮想関数ではないが、派生クラスでオーバーライドされることを想定
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ExprNode (Type: " << (type ? type->getTypeName() : "unknown") << ") {}" << std::endl;
    }
};

// 数値リテラルを表すノード
class NumberLiteralNode : public ExprNode{
public:
    int value; // 数値
    NumberLiteralNode(int val) : value(val){}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "NumberLiteralNode (Value: " << value << ", Type: " << (type ? type->getTypeName() : "unknown") << ")" << std::endl;
    }
};
// 小数リテラルを表すノード
class DecimalLiteralNode : public ExprNode{
public:
    double value; // 小数値
    DecimalLiteralNode(double val) : value(val){}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "DecimalLiteralNode (Value: " << value << ", Type: " << (type ? type->getTypeName() : "unknown") << ")" << std::endl;
    }
};
// 配列リテラルを表すノード
class ArrayLiteralNode : public ExprNode{
public:
    std::vector<std::shared_ptr<ExprNode>> elem;
    ArrayLiteralNode(std::vector<std::shared_ptr<ExprNode>> e) : elem(e){}
    void dump(int indent = 0) const override{
        printIndent(indent);
        std::cout << "ArrayLiteralNoe (Value: \n\t";
        for(auto&& i : elem){
            printIndent(indent);
            i->dump(indent + 1);
        }
        std::cout << ", Type: " << (type ? type->getTypeName() : "unknown") << ")" << std::endl;
    }
};

// 変数参照を表すノード
class VariableRefNode : public ExprNode{
public:
    std::string name; // 変数名
    std::shared_ptr<Symbol> symbol = nullptr;
    VariableRefNode(const std::string& varName) : name(varName) {}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VariableRefNode (Name: " << name << ", Type: " << (type ? type->getTypeName() : "unknown") << ")" << std::endl;
    }
};

// 配列参照を表すノード
class ArrayRefNode : public ExprNode{
public:
    std::string name; // 配列名
    std::shared_ptr<ExprNode> idx; // インデックス
    std::shared_ptr<Symbol> symbol = nullptr;
    ArrayRefNode(const std::string& arrName, std::shared_ptr<ExprNode> i) : name(arrName), idx(i) {}
    void dump(int indent = 0) const override{
        printIndent(indent);
        std::cout << "ArrayRefNode (Name: " << name << ", Type: " << (type ? type->getTypeName() : "unknown") << ")" << std::endl;
    }
};

// 二項演算子(例: a + b)を表すノード
class BinaryOpNode : public ExprNode{
public:
    std::string op; // 演算子文字列 (例: "+", "-", "==")
    std::shared_ptr<ExprNode> left; // 左オペランド
    std::shared_ptr<ExprNode> right; // 右オペランド
    BinaryOpNode(const std::string& op, std::shared_ptr<ExprNode> lhs, std::shared_ptr<ExprNode> rhs) : op(op), left(lhs), right(rhs) {}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "BinaryOpNode (Op: " << op << ", Type: " << (type ? type->getTypeName() : "unknown") << ") {" << std::endl;
        printIndent(indent + 1);
        std::cout << "Left: " << std::endl;
        left->dump(indent + 2);
        printIndent(indent + 1);
        std::cout << "Right: " << std::endl;
        right->dump(indent + 2);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};

// 関数呼び出しを表すノード
class FunctionCallNode : public ExprNode{
public:
    std::string calleeName; // 関数名
    std::vector<std::shared_ptr<ExprNode>> args; // 引数リスト
    std::shared_ptr<FuncSymbol> symbol = nullptr; // 関数シンボル
    FunctionCallNode(const std::string& name, std::vector<std::shared_ptr<ExprNode>> arguments) : calleeName(name), args(arguments) {}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FunctionCallNode (Callee: " << calleeName << ", Type: " << (type ? type->getTypeName() : "unknown") << ") {" << std::endl;
        if (!args.empty()) {
            printIndent(indent + 1);
            std::cout << "Arguments: " << std::endl;
            for (const auto& arg : args) {
                arg->dump(indent + 2);
            }
        }
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};

// キャストを表すノード
class CastNode : public ExprNode{
public:
    std::shared_ptr<ExprNode> expression; // キャスト対象の式
    CastNode(std::shared_ptr<ExprNode> expr, std::shared_ptr<TypeNode> type) : expression(expr) {this->type = type;}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "CastNode (Target Type: " << (type ? type->getTypeName() : "unknown") << ") {" << std::endl;
        printIndent(indent + 1);
        std::cout << "Expression: " << std::endl;
        expression->dump(indent + 2);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
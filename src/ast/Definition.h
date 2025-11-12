#pragma once
#include "AstNode.h"
#include "Statement.h"
#include <memory>
#include <string>
#include <vector>

// 関数・構造体などの定義の基底クラス
class DefNode : public AstNode{
public:
    // DefNodeは直接ダンプされないが、念のため実装
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "DefNode {}" << std::endl;
    }
};

// 関数定義を表すノード
class FunctionDefNode : public StatementNode{
public: 
    std::string name; // 関数名
    std::vector<std::string> parameters; // 仮引数名リスト
    std::shared_ptr<BlockNode> body; // 関数本体のブロック
    std::shared_ptr<TypeNode> returnType = nullptr; // 戻り値の型を保持
    FunctionDefNode(const std::string& n, const std::vector<std::string>& params, std::shared_ptr<BlockNode> b) : name(n), parameters(params), body(b) {}
    void dump(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FunctionDefNode (Name: " << name << ", ReturnType: " << (returnType ? returnType->getTypeName() : "unknown") << ") {" << std::endl;
        if (!parameters.empty()) {
            printIndent(indent + 1);
            std::cout << "Parameters: [";
            for (size_t i = 0; i < parameters.size(); ++i) {
                std::cout << parameters[i] << (i == parameters.size() - 1 ? "" : ", ");
            }
            std::cout << "]" << std::endl;
        }
        printIndent(indent + 1);
        std::cout << "Body: " << std::endl;
        body->dump(indent + 2);
        printIndent(indent);
        std::cout << "}" << std::endl;
    }
};
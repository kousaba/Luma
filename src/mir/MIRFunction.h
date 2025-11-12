#pragma once
#include "mir/MIRNode.h"
#include "mir/MIRTerminator.h"
#include "mir/MIRType.h"
#include "mir/MIRValue.h"
#include "mir/MIRBasicBlock.h" // MIRBasicBlock を使うのでインクルード
#include <map>
#include <string>

// 関数
class MIRFunction : public MIRNode{
public:
    std::string name; // 関数名
    std::shared_ptr<MIRType> returnType; // 戻り値の型
    std::vector<std::shared_ptr<MIRArgumentValue>> arguments; // 引数リスト
    std::vector<std::shared_ptr<MIRBasicBlock>> basicBlocks; // 基本ブロックのリスト
    std::map<std::string, std::shared_ptr<MIRType>> localVariables; // ローカル変数の型情報など (未使用)
    explicit MIRFunction(const std::string& funcName, std::shared_ptr<MIRType> retType)
        : MIRNode(NodeType::Function), name(funcName), returnType(retType) {}
    void addArgument(std::shared_ptr<MIRArgumentValue> arg){
        arguments.push_back(arg);
    }
    void addBasicBlock(std::shared_ptr<MIRBasicBlock> block){
        basicBlocks.push_back(block);
    }

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        os << "define ";
        returnType->dump(os);
        os << " @" << name << "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            arguments[i]->dump(os);
            if (i < arguments.size() - 1) {
                os << ", ";
            }
        }
        os << ") {" << std::endl;

        for (const auto& block : basicBlocks) {
            block->dump(os, indent + 1);
        }

        printMirIndent(indent);
        os << "}" << std::endl;
    }
};
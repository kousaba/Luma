#pragma once

#include "mir/MIRFunction.h"
#include "mir/MIRNode.h"

// 最上位のモジュール
class MIRModule : public MIRNode{
public:
    std::string name; // モジュール名
    std::vector<std::shared_ptr<MIRFunction>> functions; // 関数リスト
    explicit MIRModule(const std::string& moduleName = "LumaMIRModule")
        : MIRNode(NodeType::Module), name(moduleName) {}
    void addFunction(std::shared_ptr<MIRFunction> func){
        functions.push_back(func);
    }

    void dump(std::ostream& os, int indent = 0) const override {
        os << "; ModuleID = '" << name << "'" << std::endl;
        os << std::endl;
        for (const auto& func : functions) {
            func->dump(os, indent);
            os << std::endl;
        }
    }
};
#pragma once
#include "mir/MIRNode.h"
#include "mir/MIRInstruction.h"

class MIRTerminatorInstruction; // Forward declaration

// 基本ブロック
class MIRBasicBlock : public MIRNode{
public:
    std::string name; // 基本ブロック名
    std::vector<std::shared_ptr<MIRInstruction>> instructions; // 命令リスト
    std::shared_ptr<MIRTerminatorInstruction> terminator; // 終端命令
    explicit MIRBasicBlock(const std::string& blockName = "") : MIRNode(NodeType::BasicBlock), name(blockName) {}
    void addInstruction(std::shared_ptr<MIRInstruction> inst) {instructions.push_back(inst);}
    void setTerminator(std::shared_ptr<MIRTerminatorInstruction> term);

    void dump(std::ostream& os, int indent = 0) const override;
};
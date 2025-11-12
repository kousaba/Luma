#pragma once
#include "mir/MIRNode.h"
#include "mir/MIRInstruction.h"
#include "mir/MIRTerminator.h" // ここで MIRTerminator.h をインクルード

// 基本ブロック
class MIRBasicBlock : public MIRNode{
public:
    std::string name; // 基本ブロック名
    std::vector<std::shared_ptr<MIRInstruction>> instructions; // 命令リスト
    std::shared_ptr<MIRTerminatorInstruction> terminator; // 終端命令
    explicit MIRBasicBlock(const std::string& blockName = "") : MIRNode(NodeType::BasicBlock), name(blockName) {}
    void addInstruction(std::shared_ptr<MIRInstruction> inst) {instructions.push_back(inst);}
    void setTerminator(std::shared_ptr<MIRTerminatorInstruction> term) {terminator = term;}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        os << name << ":" << std::endl;
        for (const auto& inst : instructions) {
            inst->dump(os, indent + 1); // 命令は1つインデントを深くする
        }
        if (terminator) {
            terminator->dump(os, indent + 1); // 終端命令も1つインデントを深くする
        }
    }
};

// MIRBranchInstruction のコンストラクタと dump の実装
inline MIRBranchInstruction::MIRBranchInstruction(std::shared_ptr<MIRBasicBlock> target)
    : MIRTerminatorInstruction(NodeType::BranchInstruction), targetBlock(target) {}

inline void MIRBranchInstruction::dump(std::ostream& os, int indent) const {
    printMirIndent(indent);
    os << "br label %" << targetBlock->name << std::endl;
}

// MIRConditionBranchInstruction のコンストラクタと dump の実装
inline MIRConditionBranchInstruction::MIRConditionBranchInstruction(std::shared_ptr<MIRValue> cond, std::shared_ptr<MIRBasicBlock> trueB, std::shared_ptr<MIRBasicBlock> falseB)
    : MIRTerminatorInstruction(NodeType::ConditionalBranchInstruction), condition(cond), trueBlock(trueB), falseBlock(falseB) {}

inline void MIRConditionBranchInstruction::dump(std::ostream& os, int indent) const {
    printMirIndent(indent);
    os << "br ";
    condition->dump(os);
    os << ", label %" << trueBlock->name << ", label %" << falseBlock->name << std::endl;
}
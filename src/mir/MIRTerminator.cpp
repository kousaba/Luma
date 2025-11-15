#include "mir/MIRTerminator.h"
#include "mir/MIRBasicBlock.h"
#include "mir/MIRValue.h"

// MIRReturnInstruction の dump の実装
void MIRReturnInstruction::dump(std::ostream& os, int indent) const {
    printMirIndent(indent);
    os << "ret ";
    if (returnValue) {
        returnValue->dump(os);
    } else {
        os << "void";
    }
    os << std::endl;
}

// MIRBranchInstruction のコンストラクタと dump の実装
MIRBranchInstruction::MIRBranchInstruction(std::shared_ptr<MIRBasicBlock> target)
    : MIRTerminatorInstruction(NodeType::BranchInstruction), targetBlock(target) {}

void MIRBranchInstruction::dump(std::ostream& os, int indent) const {
    printMirIndent(indent);
    os << "br label %" << targetBlock->name << std::endl;
}

// MIRConditionBranchInstruction のコンストラクタと dump の実装
MIRConditionBranchInstruction::MIRConditionBranchInstruction(std::shared_ptr<MIRValue> cond, std::shared_ptr<MIRBasicBlock> trueB, std::shared_ptr<MIRBasicBlock> falseB)
    : MIRTerminatorInstruction(NodeType::ConditionalBranchInstruction), condition(cond), trueBlock(trueB), falseBlock(falseB) {}

void MIRConditionBranchInstruction::dump(std::ostream& os, int indent) const {
    printMirIndent(indent);
    os << "br ";
    condition->dump(os);
    os << ", label %" << trueBlock->name << ", label %" << falseBlock->name << std::endl;
}

// MIRBasicBlock の setTerminator の実装
void MIRBasicBlock::setTerminator(std::shared_ptr<MIRTerminatorInstruction> term) {
    terminator = term;
}

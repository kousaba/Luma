#pragma once
#include "mir/MIRInstruction.h" // MIRInstruction.h は MIRValue.h をインクルードしているはず

// 前方宣言
class MIRBasicBlock;

// 終端命令の基底クラス(制御フローを決定する命令)
class MIRTerminatorInstruction : public MIRInstruction{
public:
    explicit MIRTerminatorInstruction(NodeType type) : MIRInstruction(type) {}
    // 各派生クラスで実装される
    void dump(std::ostream& os, int indent = 0) const override = 0;
};

// リターン命令
class MIRReturnInstruction : public MIRTerminatorInstruction{
public:
    std::shared_ptr<MIRValue> returnValue; // 戻り値 (voidの場合はnullptr)
    explicit MIRReturnInstruction(std::shared_ptr<MIRValue> retVal = nullptr)
        : MIRTerminatorInstruction(NodeType::ReturnInstruction), returnValue(retVal) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        os << "ret ";
        if (returnValue) {
            returnValue->dump(os);
        } else {
            os << "void";
        }
        os << std::endl;
    }
};

// 無条件分岐命令
class MIRBranchInstruction : public MIRTerminatorInstruction{
public:
    std::shared_ptr<MIRBasicBlock> targetBlock; // 分岐先の基本ブロック
    explicit MIRBranchInstruction(std::shared_ptr<MIRBasicBlock> target);
    void dump(std::ostream& os, int indent = 0) const override; // 宣言のみ
};

// 条件分岐命令
class MIRConditionBranchInstruction : public MIRTerminatorInstruction{ // ユーザーのコードに合わせてConditionBranch
public:
    std::shared_ptr<MIRValue> condition; // 条件式の結果
    std::shared_ptr<MIRBasicBlock> trueBlock; // 条件が真の場合の分岐先
    std::shared_ptr<MIRBasicBlock> falseBlock; // 条件が偽の場合の分岐先
    explicit MIRConditionBranchInstruction(std::shared_ptr<MIRValue> cond, std::shared_ptr<MIRBasicBlock> trueB, std::shared_ptr<MIRBasicBlock> falseB);
    void dump(std::ostream& os, int indent = 0) const override; // 宣言のみ
};
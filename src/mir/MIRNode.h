#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// インデントヘルパー
inline void printMirIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

// 全てのMIRノードの基底クラス
class MIRNode{
public:
    enum class NodeType{
        Module,
        Function,
        BasicBlock,
        Instruction,
        TerminatorInstruction,
        Value,
        Type,
        LiteralType,
        RegisterValue,
        ArgumentValue,
        UnaryInstruction,
        BinaryInstruction,
        MemoryInstruction,
        CallInstruction,
        CastInstruction,
        ReturnInstruction,
        BranchInstruction,
        ConditionalBranchInstruction,
        AllocaInstruction,
        LoadInstruction,
        StoreInstruction,
        AddInstruction,
        SubInstruction,
        MulInstruction,
        DivInstruction,
        CmpInstruction,
        GepInstruction,
    };
    NodeType nodeType;
    explicit MIRNode(NodeType type) : nodeType(type) {}
    virtual ~MIRNode() = default;
    virtual void dump(std::ostream& os, int indent = 0) const = 0;
};
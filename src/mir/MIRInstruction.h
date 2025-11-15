#pragma once
#include "mir/MIRNode.h"
#include "mir/MIRValue.h"
#include <vector>
#include <map>

// 命令の基底クラス
class MIRInstruction : public MIRNode{
public:
    std::shared_ptr<MIRRegisterValue> result; // 命令の結果が格納される一時変数
    explicit MIRInstruction(NodeType type, std::shared_ptr<MIRType> resultType = nullptr, const std::string& resultName = "")
        : MIRNode(type), result(resultType ? std::make_shared<MIRRegisterValue>(resultType, resultName) : nullptr) {}
    
    // 各派生クラスで実装される
    void dump(std::ostream& os, int indent = 0) const override = 0;
};

// 単項命令
class MIRUnaryInstruction : public MIRInstruction{
public:
    std::string opcode; // "neg", "not"
    std::shared_ptr<MIRValue> operand; // オペランド
    explicit MIRUnaryInstruction(const std::string& op, std::shared_ptr<MIRValue> val, std::shared_ptr<MIRType> resultType, const std::string& resultName = "")
        : MIRInstruction(NodeType::UnaryInstruction, resultType, resultName), opcode(op), operand(val) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        if (result) { result->dump(os); os << " = "; }
        os << opcode << " ";
        operand->dump(os);
        os << std::endl;
    }
};

// 二項命令
class MIRBinaryInstruction : public MIRInstruction{
public:
    std::string opcode; // "add", "sub", "icmp eq" など
    std::shared_ptr<MIRValue> leftOperand; // 左オペランド
    std::shared_ptr<MIRValue> rightOperand; // 右オペランド
    explicit MIRBinaryInstruction(const std::string& op, std::shared_ptr<MIRValue> left, std::shared_ptr<MIRValue> right, std::shared_ptr<MIRType> resultType, const std::string& resultName = "")
        : MIRInstruction(NodeType::BinaryInstruction, resultType, resultName), opcode(op), leftOperand(left), rightOperand(right) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        if (result) { result->dump(os); os << " = "; }
        os << opcode << " ";
        leftOperand->dump(os);
        os << ", ";
        rightOperand->dump(os);
        os << std::endl;
    }
};

// メモリ割り当て命令 (alloca)
class MIRAllocaInstruction : public MIRInstruction{
public: 
    std::shared_ptr<MIRType> allocatedType; // 割り当てるメモリの型
    std::string varName; // 変数名 (デバッグ用)
    size_t size;
    explicit MIRAllocaInstruction(std::shared_ptr<MIRType> typeToAlloc, const std::string& name, std::shared_ptr<MIRType> resultPtrType, const std::string& resultName = "", size_t siz = 0)
        : MIRInstruction(NodeType::AllocaInstruction, resultPtrType, resultName), allocatedType(typeToAlloc), varName(name), size(siz) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        result->dump(os);
        os << " = alloca ";
        allocatedType->dump(os);
        if (!varName.empty()) {
            os << " ; " << varName;
        }
        os << std::endl;
    }
};

// ロード命令
class MIRLoadInstruction : public MIRInstruction{
public:
    std::shared_ptr<MIRValue> pointer; // ロード元のアドレス
    explicit MIRLoadInstruction(std::shared_ptr<MIRValue> ptr, std::shared_ptr<MIRType> resultType, const std::string& resultName = "")
        : MIRInstruction(NodeType::LoadInstruction, resultType, resultName), pointer(ptr) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        result->dump(os);
        os << " = load ";
        
        // pointer->type->name は "int*" のようになっているはずなので、'*'を取り除く
        std::string pointeeType = pointer->type->name;
        size_t starPos = pointeeType.rfind('*');
        if (starPos != std::string::npos) {
            pointeeType.erase(starPos);
        }
        os << pointeeType;

        os << ", ";
        pointer->dump(os); // ロード元ポインタ
        os << std::endl;
    }
};

// ストア命令
class MIRStoreInstruction : public MIRInstruction{
public:
    std::shared_ptr<MIRValue> value; // ストアする値
    std::shared_ptr<MIRValue> pointer; // ストア先のアドレス
    explicit MIRStoreInstruction(std::shared_ptr<MIRValue> val, std::shared_ptr<MIRValue> ptr)
        : MIRInstruction(NodeType::StoreInstruction), value(val), pointer(ptr) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        os << "store ";
        value->dump(os);
        os << ", ";
        pointer->dump(os);
        os << std::endl;
    }
};

// 関数呼び出し命令
class MIRCallInstruction : public MIRInstruction{
public:
    std::string calleeName; // 呼び出す関数名
    std::vector<std::shared_ptr<MIRValue>> arguments; // 引数リスト
    explicit MIRCallInstruction(const std::string& name, const std::vector<std::shared_ptr<MIRValue>>& args, std::shared_ptr<MIRType> resultType, const std::string& resultName = "")
        : MIRInstruction(NodeType::CallInstruction, resultType, resultName), calleeName(name), arguments(args) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        if (result) { result->dump(os); os << " = "; }
        os << "call @" << calleeName << "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            arguments[i]->dump(os);
            if (i < arguments.size() - 1) {
                os << ", ";
            }
        }
        os << ")" << std::endl;
    }
};

// キャスト命令のオペコード
enum class CastOpcode {
    SIToFP,   // 符号付き整数 -> 浮動小数点数
    FPToSI,   // 浮動小数点数 -> 符号付き整数
    IntCast,  // 整数同士のキャスト (bitcast, trunc, sext, zext)
    FPCast,   // 浮動小数点数同士のキャスト
    PtrToInt, // ポインタ -> 整数
    IntToPtr, // 整数 -> ポインタ
    PtrCast,  // ポインタ同士のキャスト
};

// キャスト命令
class MIRCastInstruction : public MIRInstruction{
public:
    CastOpcode opcode;
    std::shared_ptr<MIRValue> operand; // キャスト対象のオペランド
    std::shared_ptr<MIRType> targetType; // キャスト先の型
    explicit MIRCastInstruction(CastOpcode op, std::shared_ptr<MIRValue> val, std::shared_ptr<MIRType> target, const std::string& resultName = "")
        : MIRInstruction(NodeType::CastInstruction, target, resultName), opcode(op), operand(val), targetType(target) {}

    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        result->dump(os);
        os << " = ";
        switch(opcode) {
            case CastOpcode::SIToFP: os << "sitofp"; break;
            case CastOpcode::FPToSI: os << "fptosi"; break;
            case CastOpcode::IntCast: os << "intcast"; break;
            case CastOpcode::FPCast: os << "fpcast"; break;
            case CastOpcode::PtrToInt: os << "ptrtoint"; break;
            case CastOpcode::IntToPtr: os << "inttoptr"; break;
            case CastOpcode::PtrCast: os << "ptrcast"; break;
        }
        os << " ";
        operand->dump(os);
        os << " to ";
        targetType->dump(os);
        os << std::endl;
    }
};

// GEP命令
class MIRGepInstruction : public MIRInstruction{
public:
    std::shared_ptr<MIRValue> basePtr;
    std::shared_ptr<MIRValue> index;
    std::shared_ptr<MIRType> elementType; // これが要素の型 (int)
    std::shared_ptr<MIRType> ptrOrArrayType; // basePtrが指す型 ([5 x i64])
    
    explicit MIRGepInstruction(
        std::shared_ptr<MIRValue> base,
        std::shared_ptr<MIRValue> idx,
        std::shared_ptr<MIRType> elemType,
        std::shared_ptr<MIRType> ptrOrArrayTy, // 新しい引数
        const std::string& resultName
    ) : MIRInstruction(NodeType::GepInstruction, elemType, resultName),
        basePtr(base), 
        index(idx), 
        elementType(elemType),
        ptrOrArrayType(ptrOrArrayTy) {} // 新しいメンバの初期化
    
    void dump(std::ostream& os, int indent = 0) const override {
        printMirIndent(indent);
        if (result) { result->dump(os); os << " = "; }
        os << "getelementptr " << elementType->name 
            << ", ptr ";
        basePtr->dump(os);
        os << ", ";
        index->dump(os);
        os << std::endl;
    }
};
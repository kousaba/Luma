#pragma once
#include "mir/MIRNode.h"
#include "mir/MIRType.h"
#include <string>

// MIRにおける値の基底クラス
// レジスタ、リテラル、引数などが継承
class MIRValue : public MIRNode{
public:
    std::shared_ptr<MIRType> type; // 値の型
    std::string name; // 値の名前 (例: %0, %var)
    explicit MIRValue(NodeType type, std::shared_ptr<MIRType> valueType, const std::string& valName = "") : MIRNode(type), type(valueType), name(valName) {}
    ~MIRValue() override = default;

    void dump(std::ostream& os, int indent = 0) const override {
        type->dump(os);
        os << " " << name;
    }
};

// 定数リテラルの値
class MIRLiteralValue : public MIRValue{
public:
    std::string stringValue; // 値の文字列表現 (例: "3.14", "15", "true")
    explicit MIRLiteralValue(std::shared_ptr<MIRType> valueType, const std::string& val) : MIRValue(NodeType::LiteralType, valueType, ""), stringValue(val) {}

    void dump(std::ostream& os, int indent = 0) const override {
        type->dump(os);
        os << " " << stringValue;
    }
};

// 命令の結果を格納する一時変数（レジスタ）
class MIRRegisterValue : public MIRValue{
public:
    explicit MIRRegisterValue(std::shared_ptr<MIRType> valueType, const std::string& regName) : MIRValue(NodeType::RegisterValue, valueType, regName) {}
};

// 関数の引数
class MIRArgumentValue : public MIRValue{
public:
    size_t argIndex; // 引数のインデックス
    explicit MIRArgumentValue(std::shared_ptr<MIRType> argType, const std::string& argName, size_t index) : MIRValue(NodeType::ArgumentValue, argType, argName), argIndex(index) {}
};
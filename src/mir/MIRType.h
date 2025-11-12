#pragma once
#include "mir/MIRNode.h"

// MIRの型情報を表すクラス
class MIRType : public MIRNode{
public:
    enum class TypeID{
        Void,       // 戻り値なし
        Bool,       // 真偽値
        Int,        // 整数型
        Float,      // 浮動小数点型
        Ptr,        // ポインタ型
        Struct,     // 構造体型
        Function,   // 関数型
        Array,      // 配列型
        Unknown     // 不明な型
    };
    TypeID id; // 型の識別子
    std::string name; // 型名("int", "float", "i32", "f32"など)
    explicit MIRType(TypeID typeId, const std::string& typeName = "") : MIRNode(NodeType::Type), id(typeId), name(typeName){}
    bool isInteger() const {return id == TypeID::Int;}
    bool isFloat() const {return id == TypeID::Float;}
    bool isPointer() const {return id == TypeID::Ptr;}
    bool isVoid() const {return id == TypeID::Void;}
    bool isBool() const {return id == TypeID::Bool;}
    bool isStruct() const {return id == TypeID::Struct;}
    bool isFunction() const {return id == TypeID::Function;}
    bool isArray() const {return id == TypeID::Array;}
    bool isUnknown() const {return id == TypeID::Unknown;}

    void dump(std::ostream& os, int indent = 0) const override {
        os << name;
    }
};
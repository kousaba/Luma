#pragma once
#include <string>
#include <memory>
#include <vector>

// すべての型の基底クラス
class TypeNode : public std::enable_shared_from_this<TypeNode> {
public:
    virtual ~TypeNode() = default;
    virtual std::string getTypeName() const = 0;
    virtual bool isInteger() const { return false; }
    virtual bool isFloat() const { return false; }
    virtual bool isNumeric() const { return isInteger() || isFloat(); }
};

// 基本的な型を表すノード
class BasicTypeNode: public TypeNode{
public:
    std::string name;
    BasicTypeNode(const std::string& n) : name(n){};
    std::string getTypeName() const override {return name;}
    bool isInteger() const override { return name == "int" || name == "int32" || name == "i32"; }
    bool isFloat() const override { return name == "float" || name == "f32"; }
};
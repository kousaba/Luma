#pragma once

#include "types/Type.h"
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <llvm/IR/Value.h>
#include <memory>

class Symbol;
class Scope;

enum class SymbolKind{
    VAR,
    FUNC,
    TYPE
};

class Symbol{
public:
    SymbolKind kind;
    std::string name;
    std::shared_ptr<TypeNode> type; // 関数の戻り値・変数の型
    std::shared_ptr<Scope> scope;
    Symbol(const std:: string& name, SymbolKind kind, std::shared_ptr<TypeNode> type, std::shared_ptr<Scope> scope)
        : name(name), kind(kind), type(type), scope(scope) {}
    virtual ~Symbol() = default;
    virtual void dump(std::ostream& os, int indent = 0) const{
        os << std::string(indent, ' ') << name << "(Kind: " << (int)kind << ", Type: " << 
        (type ? type->getTypeName() : "N/A") << ")";
    }
};

class VarSymbol : public Symbol{
public:
    VarSymbol(const std::string& name, std::shared_ptr<TypeNode> type, std::shared_ptr<Scope> scope)
        : Symbol(name, SymbolKind::VAR, type, scope) {}
    void dump(std::ostream& os, int indent = 0) const override {
        os << std::string(indent, ' ') << "VarSymbol: " << name << " (Type: " << 
        (type ? type->getTypeName() : "N/A") << ")";
    }
};

class FuncSymbol : public Symbol{
public:
    std::vector<std::shared_ptr<VarSymbol>> parameters;
    std::shared_ptr<Scope> funcScope; // 関数が作るスコープ
    FuncSymbol(const std::string& name, std::shared_ptr<TypeNode> returnType, std::shared_ptr<Scope> scope)
        : Symbol(name, SymbolKind::FUNC, returnType, scope) {}
    void addParameter(std::shared_ptr<VarSymbol> param) {parameters.push_back(param);}
    void dump(std::ostream& os, int indent = 0) const override{
        os << std::string(indent, ' ') << "FuncSymbol: " << name << " (ReturnType: " << 
        (type ? type->getTypeName() : "N/A") << ")" << std::endl;
        for(const auto& param : parameters){
            os << std::string(indent + 2, ' ');
            param->dump(os);
            os << std::endl;
        }
    }
};

class TypeSymbol : public Symbol{
public:
    TypeSymbol(const std::string& name, std::shared_ptr<TypeNode> type, std::shared_ptr<Scope> scope)
        : Symbol(name, SymbolKind::TYPE, type, scope) {}
    void dump(std::ostream& os, int indent = 0) const override{
        os << std::string(indent, ' ') << "TypeSymbol: " << name << " (Represents: " <<
        (type ? type->getTypeName() : "N/A") << ")";
    }
};

class Scope : public std::enable_shared_from_this<Scope>{
public:
    std::shared_ptr<Scope> parent; // 親スコープ
    std::map<std::string, std::shared_ptr<Symbol>> symbols; // このスコープに定義されたシンボル
    Scope(std::shared_ptr<Scope> parent = nullptr) : parent(parent) {}
    bool define(std::shared_ptr<Symbol> sym);
    std::shared_ptr<Symbol> lookup(const std::string& name);
    std::shared_ptr<Symbol> lookupCurrent(const std::string& name);
    void dump(std::ostream& os, int indent = 0) const;
};
#pragma once

#include <string>
#include <vector>
#include <llvm/IR/Value.h>
#include <memory>

// Forward declaration
class TypeNode;

enum class SymbolKind{
    VAR,
    FUNC,
    STRUCT
};

struct Symbol : public std::enable_shared_from_this<Symbol> {
    SymbolKind kind;
    std::string name;
    TypeNode* type;
    llvm::Value* llvmValue = nullptr;
    std::vector<TypeNode*> argTypes;
    
    // Constructors
    Symbol(const std::string& name, TypeNode* type);
    Symbol(const std::string& name, TypeNode* returnType, const std::vector<TypeNode*>& args);
    Symbol() = delete;
};

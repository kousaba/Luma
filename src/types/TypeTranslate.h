#pragma once
#include "types/Type.h"
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>

class TypeTranslate{
public:
    static llvm::Type* toLlvmType(TypeNode* type, llvm::LLVMContext& context);
};
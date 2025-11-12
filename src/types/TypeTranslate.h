#pragma once
#include "mir/MIRType.h"
#include "types/Type.h"
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>

class TypeTranslate{
public:
    static llvm::Type* toLlvmType(TypeNode* type, llvm::LLVMContext& context);
    static llvm::Type* toLlvmType(MIRType* mirType, llvm::LLVMContext& context);
    static std::shared_ptr<MIRType> toMirType(TypeNode* type);
};
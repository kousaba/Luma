#include "TypeTranslate.h"
#include <llvm-18/llvm/IR/IRBuilder.h>

llvm::Type* TypeTranslate::toLlvmType(TypeNode *type, llvm::LLVMContext &context){
    if(!type) return nullptr;
    std::string typeName = type->getTypeName();
    auto builder = std::make_unique<llvm::IRBuilder<>>(context);
    if(typeName == "int") return builder->getInt64Ty();
    if(typeName == "i32") return builder->getInt32Ty();
    if(typeName == "char") return builder->getInt8Ty();
    if(typeName == "float") return builder->getDoubleTy();
    if(typeName == "f32") return builder->getFloatTy();
    if(typeName == "void") return builder->getVoidTy();
    if(typeName == "bool") return builder->getInt1Ty();
    // TODO: 構造体などに対応
    return nullptr;
}


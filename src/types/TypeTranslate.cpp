#include "TypeTranslate.h"
#include "mir/MIRType.h"
#include "types/Type.h"
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-18/llvm/IR/IRBuilder.h>
#include <memory>

llvm::Type* TypeTranslate::toLlvmType(TypeNode *type, llvm::LLVMContext &context){
    if(!type) return nullptr;
    std::string typeName = type->getTypeName();
    if(typeName == "int") return llvm::Type::getInt64Ty(context);
    if(typeName == "i32") return llvm::Type::getInt32Ty(context);
    if(typeName == "char") return llvm::Type::getInt8Ty(context);
    if(typeName == "float") return llvm::Type::getDoubleTy(context);
    if(typeName == "f32") return llvm::Type::getFloatTy(context);
    if(typeName == "void") return llvm::Type::getVoidTy(context);
    if(typeName == "bool") return llvm::Type::getInt1Ty(context);
    // TODO: 構造体などに対応
    return nullptr;
}

llvm::Type* TypeTranslate::toLlvmType(MIRType* mirType, llvm::LLVMContext &context){
    if(!mirType) return llvm::Type::getVoidTy(context);
    switch(mirType->id){
        case MIRType::TypeID::Int:{
            if(mirType->name == "char") return llvm::Type::getInt8Ty(context);
            if(mirType->name == "bool") return llvm::Type::getInt1Ty(context);
            if(mirType->name == "i32") return llvm::Type::getInt32Ty(context);
            if(mirType->name == "i64") return llvm::Type::getInt64Ty(context);
            if(mirType->name == "int") return llvm::Type::getInt64Ty(context);
            return llvm::Type::getInt64Ty(context);
        }
        case MIRType::TypeID::Float:{
            if(mirType->name == "f32") return llvm::Type::getFloatTy(context);
            if(mirType->name == "float") return llvm::Type::getDoubleTy(context);
            return llvm::Type::getDoubleTy(context);
        }
        case MIRType::TypeID::Bool:
            return llvm::Type::getInt1Ty(context);
        case MIRType::TypeID::Void:
            return llvm::Type::getVoidTy(context);
        case MIRType::TypeID::Struct:{
            if(mirType->name.empty()){
                return llvm::Type::getInt8Ty(context);
            }
            llvm::StructType* st = llvm::StructType::getTypeByName(context, mirType->name);
            if(!st){
                st = llvm::StructType::create(context, mirType->name);
            }
            return st;
        }
        case MIRType::TypeID::Ptr:
            // TODO: 型名をつけたポインタを返すようにする
            return llvm::PointerType::getInt8Ty(context);
        case MIRType::TypeID::Array: {
            llvm::Type* llvmElementType = toLlvmType(mirType->elementType.get(), context);
            if (!llvmElementType) {
                // エラーハンドリング
                return llvm::Type::getVoidTy(context);
            }
            return llvm::ArrayType::get(llvmElementType, mirType->arraySize);
        }
        case MIRType::TypeID::Function:
            // 詳細情報を入れる
            return llvm::FunctionType::get(llvm::Type::getVoidTy(context), false)->getPointerTo();
        case MIRType::TypeID::Unknown:
        default:
            return llvm::Type::getVoidTy(context);
    }
}

std::shared_ptr<MIRType> TypeTranslate::toMirType(TypeNode* typeNode){
    if(!typeNode){
        return std::make_shared<MIRType>(MIRType::TypeID::Void);
    }
    std::string typeName = typeNode->getTypeName();

    // 配列型の場合の処理 (例: "int[5]")
    size_t bracketPos = typeName.find('[');
    if (bracketPos != std::string::npos) {
        std::string baseTypeName = typeName.substr(0, bracketPos);
        std::string sizeStr = typeName.substr(bracketPos + 1, typeName.length() - bracketPos - 2);
        size_t arraySize = 0;
        try {
            arraySize = std::stoul(sizeStr);
        } catch (const std::exception& e) {
            // エラーハンドリング
            return std::make_shared<MIRType>(MIRType::TypeID::Unknown);
        }

        // 配列の要素型を再帰的に取得
        std::shared_ptr<MIRType> elemType = nullptr;
        if(baseTypeName == "int") elemType = std::make_shared<MIRType>(MIRType::TypeID::Int, "int");
        else if(baseTypeName == "i32") elemType = std::make_shared<MIRType>(MIRType::TypeID::Int, "i32");
        else if(baseTypeName == "float") elemType = std::make_shared<MIRType>(MIRType::TypeID::Float, "float");
        else if(baseTypeName == "f32") elemType = std::make_shared<MIRType>(MIRType::TypeID::Float, "f32");
        else if(baseTypeName == "bool") elemType = std::make_shared<MIRType>(MIRType::TypeID::Bool, "bool");
        else if(baseTypeName == "void") elemType = std::make_shared<MIRType>(MIRType::TypeID::Void, "void");

        if (elemType) {
            return std::make_shared<MIRType>(elemType, arraySize); // 配列型コンストラクタを使用
        }
    }

    // 基本型の処理
    if(typeName == "int") return std::make_shared<MIRType>(MIRType::TypeID::Int, "int");
    if(typeName == "i32") return std::make_shared<MIRType>(MIRType::TypeID::Int, "i32");
    if(typeName == "float") return std::make_shared<MIRType>(MIRType::TypeID::Float, "float");
    if(typeName == "f32") return std::make_shared<MIRType>(MIRType::TypeID::Float, "f32");
    if(typeName == "bool") return std::make_shared<MIRType>(MIRType::TypeID::Bool, "bool");
    if(typeName == "void") return std::make_shared<MIRType>(MIRType::TypeID::Void, "void");

    if(auto* arrayType = dynamic_cast<ArrayTypeNode*>(typeNode)){
        auto elemType = toMirType(arrayType->arrType.get());
        size_t size = arrayType->size;
        return std::make_shared<MIRType>(elemType, size);
    }
    // ポインタ用?
    // if(auto* ptrType = dynamic_cast<PointerTypeNode*>(typeNode)) {
    //     auto pointeeType = toMirType(ptrType->pointeeType);
    //     return std::make_shared<MIRType>(pointeeType);
    // }
    return std::make_shared<MIRType>(MIRType::TypeID::Unknown);
}

std::shared_ptr<TypeNode> TypeTranslate::toTypeNode(const std::string& typeName){
    if(typeName == "int" || typeName == "i32" || typeName == "char" || 
       typeName == "float" || typeName == "f32" || typeName == "bool" ||
       typeName == "void") return std::make_shared<BasicTypeNode>(typeName);
    return nullptr;
}
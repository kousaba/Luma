#include "LLVMGen.h"
#include "common/ErrorHandler.h"
#include "mir/MIRBasicBlock.h"
#include "mir/MIRFunction.h"
#include "mir/MIRInstruction.h"
#include "mir/MIRTerminator.h"
#include "mir/MIRType.h"
#include "mir/MIRValue.h"
#include "semantic/SemanticAnalysis.h"
#include "types/TypeTranslate.h"
#include <llvm-18/llvm/ADT/StringRef.h>
#include <llvm-18/llvm/IR/BasicBlock.h>
#include <llvm-18/llvm/IR/Constants.h>
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

LLVMGen::LLVMGen(SemanticAnalysis& sema) : context(std::make_unique<llvm::LLVMContext>()), module(std::make_unique<llvm::Module>("LumaModule", *context)),
                    builder(std::make_unique<llvm::IRBuilder<>>(*context)), semanticAnalysis(sema){}

llvm::Module* LLVMGen::generate(MIRModule *mirModule){
    for(auto& func : mirModule->functions){
        visit(func.get());
    }
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    if(llvm::verifyModule(*module, &errorStream)){
        return nullptr;
    }
    return module.get();
}

llvm::Module* LLVMGen::getModule(){
    return module.get();
}

std::unique_ptr<llvm::Module> LLVMGen::releaseModule(){
    return std::move(module);
}

void LLVMGen::visit(MIRFunction *node){
    // 関数の型を生成
    std::vector<llvm::Type*> args;
    for(auto& param : node->arguments){
        args.push_back(TypeTranslate::toLlvmType(param->type.get(), *context));
    }

    llvm::Type* retType = TypeTranslate::toLlvmType(node->returnType.get(), *context);

    llvm::FunctionType* funcType = llvm::FunctionType::get(
        retType,
        args,
        false
    );

    llvm::Function* llvmFunc = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node->name,
        module.get()
    );

    // 引数のマッピングを追加
    auto mirArgs = node->arguments.begin();
    auto llvmArgs = llvmFunc->arg_begin();
    while (mirArgs != node->arguments.end() && llvmArgs != llvmFunc->arg_end()) {
        valueMap[mirArgs->get()] = &(*llvmArgs);
        llvmArgs->setName((*mirArgs)->name);
        mirArgs++;
        llvmArgs++;
    }

    currentFunction = llvmFunc;

    for(auto& block : node->basicBlocks){
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, block->name, currentFunction);
        blockMap[block.get()] = bb;
    }

    for(auto& block : node->basicBlocks){
        visit(block.get());
    }
}

void LLVMGen::visit(MIRBasicBlock *node){
    auto bb = blockMap[node];
    builder->SetInsertPoint(bb);
    for(auto &instr : node->instructions){
        visit(instr.get());
    }
    if(node->terminator){
        visit(node->terminator.get());
    }
}

void LLVMGen::visit(MIRInstruction *node){
    if(auto allocaInst = dynamic_cast<MIRAllocaInstruction*>(node)) return visit(allocaInst);
    if(auto storeInst = dynamic_cast<MIRStoreInstruction*>(node)) return visit(storeInst);
    if(auto loadInst = dynamic_cast<MIRLoadInstruction*>(node)) return visit(loadInst);
    if(auto binaryInst = dynamic_cast<MIRBinaryInstruction*>(node)) return visit(binaryInst);
    if(auto unaryInst = dynamic_cast<MIRUnaryInstruction*>(node)) return visit(unaryInst);
    if(auto castInst = dynamic_cast<MIRCastInstruction*>(node)) {visit(castInst); return;}
    if(auto callInst = dynamic_cast<MIRCallInstruction*>(node)) return visit(callInst);
    errorHandler.errorReg("Unhandled MIRInstruction type: " + std::string(typeid(*node).name()), 0);
}

void LLVMGen::visit(MIRAllocaInstruction *node){
    llvm::Type* allocType = TypeTranslate::toLlvmType(node->allocatedType.get(), *context);
    llvm::Value* allocaVal;
    if(node->size > 0){
        allocaVal = builder->CreateAlloca(
            allocType,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), node->size),
            node->varName
        );
    }else{
        allocaVal = builder->CreateAlloca(
            allocType,
            nullptr,
            node->varName
        );
    }
    MIRValue* mirResultValue = node->result.get();
    valueMap[mirResultValue] = allocaVal;
}

void LLVMGen::visit(MIRStoreInstruction *node){
    llvm::Value* value = visit(node->value.get());
    llvm::Value* ptr = visit(node->pointer.get());
    if(!value || !ptr){
        errorHandler.errorReg("store ptr or value nullptr", -1);
        return;
    }
    builder->CreateStore(value, ptr);
}

void LLVMGen::visit(MIRLoadInstruction *node){
    llvm::Type* type = TypeTranslate::toLlvmType(node->result->type.get(), *context);
    llvm::Value* ptr = visit(node->pointer.get());
    llvm::Value* loadedValue = builder->CreateLoad(type, ptr, "loadtmp");
    valueMap[node->result.get()] = loadedValue;
}

void LLVMGen::visit(MIRBinaryInstruction *node){
    llvm::Value* left = visit(node->leftOperand.get());
    llvm::Value* right = visit(node->rightOperand.get());
    llvm::Value* resultValue = nullptr;
    if(node->leftOperand->type.get()->isInteger()){
        if(node->opcode == "add") resultValue = builder->CreateAdd(left, right, "addtmp");
        if(node->opcode == "sub") resultValue = builder->CreateSub(left, right, "subtmp");
        if(node->opcode == "mul") resultValue = builder->CreateMul(left, right, "multmp");
        if(node->opcode == "div") resultValue = builder->CreateSDiv(left,right, "divtmp");
        if(node->opcode == "icmp eq") resultValue = builder->CreateICmpEQ(left, right, "eqtmp");
        if(node->opcode == "icmp ne") resultValue = builder->CreateICmpNE(left, right, "netmp");
        if(node->opcode == "icmp lt") resultValue = builder->CreateICmpSLT(left, right, "lttmp");
        if(node->opcode == "icmp gt") resultValue = builder->CreateICmpSGT(left, right, "gttmp");
        if(node->opcode == "icmp le") resultValue = builder->CreateICmpSLE(left, right, "letmp");
        if(node->opcode == "icmp ge") resultValue = builder->CreateICmpSGE(left, right, "getmp");
    }else if(node->leftOperand->type.get()->isFloat()){
        if(node->opcode == "add") resultValue = builder->CreateFAdd(left, right, "faddtmp");
        if(node->opcode == "sub") resultValue = builder->CreateFSub(left, right, "fsubtmp");
        if(node->opcode == "mul") resultValue = builder->CreateFMul(left, right, "fmultmp");
        if(node->opcode == "div") resultValue = builder->CreateFDiv(left,right, "fdivtmp");
        if(node->opcode == "fcmp eq") resultValue = builder->CreateFCmpOEQ(left, right, "feqtmp");
        if(node->opcode == "fcmp ne") resultValue = builder->CreateFCmpONE(left, right, "fnetmp");
        if(node->opcode == "fcmp lt") resultValue = builder->CreateFCmpOLT(left, right, "flttmp");
        if(node->opcode == "fcmp gt") resultValue = builder->CreateFCmpOGT(left, right, "fgttmp");
        if(node->opcode == "fcmp le") resultValue = builder->CreateFCmpOLE(left, right, "fletmp");
        if(node->opcode == "fcmp ge") resultValue = builder->CreateFCmpOGE(left, right, "fgetmp");
    }
    if(resultValue){
        valueMap[node->result.get()] = resultValue;
    }
}

void LLVMGen::visit(MIRUnaryInstruction *node){
    llvm::Value* operand = visit(node->operand.get());
    llvm::Value* resultValue = nullptr;
    if(node->opcode == "neg"){
        if(node->operand->type.get()->isInteger()) resultValue = builder->CreateNeg(operand, "negtmp");
        else if(node->operand->type.get()->isFloat()) resultValue = builder->CreateFNeg(operand, "fnegtmp");
    }
    if(node->opcode == "not"){
        if(node->operand->type.get()->isInteger()) resultValue = builder->CreateNot(operand, "nottmp");
        else if(node->operand->type.get()->isFloat()){
            // TODO: セマンティック解析でエラーを出す
            return;
        }
    }
    if(resultValue) valueMap[node->result.get()] = resultValue;
}

void LLVMGen::visit(MIRTerminatorInstruction *node){
    if(auto retInst = dynamic_cast<MIRReturnInstruction*>(node)) return visit(retInst);
    if(auto branchInst = dynamic_cast<MIRBranchInstruction*>(node)) return visit(branchInst);
    if(auto condBranchInst = dynamic_cast<MIRConditionBranchInstruction*>(node)) return visit(condBranchInst);
}

void LLVMGen::visit(MIRReturnInstruction *node){
    if(node->returnValue){
        auto retVal = visit(node->returnValue.get());
        if(retVal){
            builder->CreateRet(retVal);
        }else{
            errorHandler.errorReg("Failed to generate return value in LLVMGen.", 0);
            builder->CreateRetVoid();
        }
    }else{
        builder->CreateRetVoid();
    }
}

void LLVMGen::visit(MIRBranchInstruction *node){
    auto targetBlock = blockMap[node->targetBlock.get()];
    if(!targetBlock) return;
    builder->CreateBr(targetBlock);
}

void LLVMGen::visit(MIRConditionBranchInstruction *node){
    auto trueBlock = blockMap[node->trueBlock.get()];
    auto falseBlock = blockMap[node->falseBlock.get()];
    llvm::Value* cond = visit(node->condition.get());
    builder->CreateCondBr(cond, trueBlock, falseBlock);
}

llvm::Value* LLVMGen::visit(MIRGepInstruction *node){
    llvm::Value* basePtr = valueMap[node->basePtr.get()];
    llvm::Value* index = valueMap[node->index.get()];
    if(!basePtr || !index){
        errorHandler.errorReg("GEP instruction: invalid operands", 0);
        return nullptr;
    }
    llvm::Type* elemType = TypeTranslate::toLlvmType(node->elementType.get(), *context);
    if(!elemType){
        errorHandler.errorReg("GEP instruction: invalid element type", 0);
        return nullptr;
    }
    llvm::Value* gepResult = builder->CreateGEP(
        elemType,
        basePtr,
        {index}
    );
    valueMap[node->result.get()] = gepResult;
    return gepResult;
}

llvm::Value* LLVMGen::visit(MIRValue *node){
    if(valueMap.count(node)) return valueMap[node];

    if(auto literal = dynamic_cast<MIRLiteralValue*>(node)) return visit(literal);
    if(auto cast = dynamic_cast<MIRCastInstruction*>(node)) return visit(cast);

    errorHandler.errorReg("Could not find or generate LLVM value for MIRValue.", 0);
    return nullptr;
}

llvm::Value* LLVMGen::visit(MIRLiteralValue *node){
    // llvm::Type* type = TypeTranslate::toLlvmType(node->type.get(), *context);

    // std::string type_str;
    // llvm::raw_string_ostream rso(type_str);
    // type->print(rso);
    // errorHandler.errorReg("Visiting MIRLiteralValue. Type is: " + rso.str(), 2);
    // if(type->isIntegerTy()){
    //     if(auto* intType = llvm::dyn_cast<llvm::IntegerType>(type)){
    //         return llvm::ConstantInt::get(intType, llvm::StringRef(node->stringValue), 10);
    //     }
    //     errorHandler.errorReg("Failed to cast to IntegerType in LLVMGen.", 0);
    //     return nullptr;
    // }
    // if(type->isFloatingPointTy()) return llvm::ConstantFP::get(type, llvm::StringRef(node->stringValue));
    // // TODO: boolなど
    // return nullptr;
    llvm::Type*  type = TypeTranslate::toLlvmType(node->type.get(), *context);
    if(!type){
        errorHandler.errorReg("Failed to translate literal type", 0);
        return nullptr;
    }
    if(type->isIntegerTy()){
        auto* intType = llvm::cast<llvm::IntegerType>(type);
        unsigned bits = intType->getBitWidth();
        llvm::APInt api(bits, llvm::StringRef(node->stringValue), 10);
        return llvm::ConstantInt::get(*context, api);
    }
    if(type->isFloatingPointTy()){
        double d = 0.0;
        try{
            d = std::stod(node->stringValue);
        }catch(...){
            errorHandler.errorReg("Invalid floating literal: " + node->stringValue, 0);
            return nullptr;
        }
        llvm::APFloat apf(d);
        return llvm::ConstantFP::get(*context, apf);
    }
    if(node->stringValue == "true") return llvm::ConstantInt::getTrue(*context);
    if(node->stringValue == "false") return llvm::ConstantInt::getFalse(*context);
    errorHandler.errorReg("Unsupported literal type for: " + node->stringValue, 0);
    return nullptr;
}

llvm::Value* LLVMGen::visit(MIRCastInstruction *node){
    llvm::Value* operand = visit(node->operand.get());
    if(!operand){
        errorHandler.errorReg("Cast operand not generated", 0);
        return nullptr;
    }

    llvm::Type* targetType = TypeTranslate::toLlvmType(node->targetType.get(), *context);
    if(!targetType){
        errorHandler.errorReg("Failed to translate cast target type", 0);
        return nullptr;
    }

    llvm::Type* operandType = operand->getType();
    llvm::Value* castVal = nullptr;

    if(operandType->isIntegerTy()){
        if(targetType->isIntegerTy()) castVal = builder->CreateIntCast(operand, targetType, true, "casttmp");
        else if(targetType->isFloatingPointTy()) castVal = builder->CreateSIToFP(operand, targetType, "casttmp");
        else if(targetType->isPointerTy()) castVal = builder->CreateIntToPtr(operand, targetType, "inttoptrtmp");
    } else if(operandType->isFloatingPointTy()){
        if(targetType->isIntegerTy()) castVal = builder->CreateFPToSI(operand, targetType, "casttmp");
        else if(targetType->isFloatingPointTy()) castVal = builder->CreateFPCast(operand, targetType, "casttmp");
    } else if(operandType->isPointerTy()){
        if(targetType->isPointerTy()) castVal = builder->CreatePointerCast(operand, targetType, "ptrcasttmp");
        else if(targetType->isIntegerTy()) castVal = builder->CreatePtrToInt(operand, targetType, "ptrtointtmp");
    }

    if(!castVal){
        errorHandler.errorReg("Unsupported cast from " + std::string(operandType->getTypeID() == llvm::Type::IntegerTyID ? "int" : "other")
            + " to target", 0);
        return nullptr;
    }
    if(node->result && node->result.get()){
        valueMap[node->result.get()] = castVal;
    }

    return castVal;
}

void LLVMGen::visit(MIRCallInstruction *node) {
    llvm::Function* calleeFunc = module->getFunction(node->calleeName);
    if (!calleeFunc) {
        // TODO: printf/scanfのような外部関数をここで宣言する
        errorHandler.errorReg("LLVMGen: Function " + node->calleeName + " not found in module.", 0);
        return;
    }

    std::vector<llvm::Value*> llvmArgs;
    for (const auto& arg : node->arguments) {
        llvmArgs.push_back(visit(arg.get()));
    }

    llvm::Value* callResult = builder->CreateCall(calleeFunc, llvmArgs, "calltmp");

    if (node->result) {
        valueMap[node->result.get()] = callResult;
    }
}

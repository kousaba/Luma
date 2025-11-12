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
    errorHandler.errorReg("Unhandled MIRInstruction type: " + std::string(typeid(*node).name()), 0);
}

void LLVMGen::visit(MIRAllocaInstruction *node){
    llvm::Type* allocType = TypeTranslate::toLlvmType(node->allocatedType.get(), *context);
    llvm::Value* allocaVal = builder->CreateAlloca(
        allocType,
        nullptr,
        node->varName
    );
    MIRValue* mirResultValue = node->result.get();
    valueMap[mirResultValue] = allocaVal;
}

void LLVMGen::visit(MIRStoreInstruction *node){
    llvm::Value* value = visit(node->value.get());
    llvm::Value* ptr = visit(node->pointer.get());
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
        if(node->opcode == "icmp eq") resultValue = builder->CreateFCmpOEQ(left, right, "feqtmp");
        if(node->opcode == "icmp ne") resultValue = builder->CreateFCmpONE(left, right, "fnetmp");
        if(node->opcode == "icmp lt") resultValue = builder->CreateFCmpOLT(left, right, "flttmp");
        if(node->opcode == "icmp gt") resultValue = builder->CreateFCmpOGT(left, right, "fgttmp");
        if(node->opcode == "icmp le") resultValue = builder->CreateFCmpOLE(left, right, "fletmp");
        if(node->opcode == "icmp ge") resultValue = builder->CreateFCmpOGE(left, right, "fgetmp");
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

llvm::Value* LLVMGen::visit(MIRValue *node){
    if(auto literal = dynamic_cast<MIRLiteralValue*>(node)) return visit(literal);
    if(auto cast = dynamic_cast<MIRCastInstruction*>(node)) return visit(cast);

    if(valueMap.count(node)) return valueMap[node];
    errorHandler.errorReg("Count not fin or generate LLVM value for MIRValue.", 0);
    return nullptr;
}

llvm::Value* LLVMGen::visit(MIRLiteralValue *node){
    llvm::Type* type = TypeTranslate::toLlvmType(node->type.get(), *context);

    std::string type_str;
    llvm::raw_string_ostream rso(type_str);
    type->print(rso);
    errorHandler.errorReg("Visiting MIRLiteralValue. Type is: " + rso.str(), 2);
    if(type->isIntegerTy()){
        if(auto* intType = llvm::dyn_cast<llvm::IntegerType>(type)){
            return llvm::ConstantInt::get(intType, llvm::StringRef(node->stringValue), 10);
        }
        errorHandler.errorReg("Failed to cast to IntegerType in LLVMGen.", 0);
        return nullptr;
    }
    if(type->isFloatingPointTy()) return llvm::ConstantFP::get(type, llvm::StringRef(node->stringValue));
    // TODO: boolなど
    return nullptr;
}

llvm::Value* LLVMGen::visit(MIRCastInstruction *node){
    auto operand = visit(node->operand.get());
    auto targetType = TypeTranslate::toLlvmType(node->targetType.get(), *context);
    auto operandType = operand->getType();
    // TODO: 実装
    return nullptr;
}
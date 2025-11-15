#include "mir/MIRBasicBlock.h"
#include "mir/MIRFunction.h"
#include "mir/MIRInstruction.h"
#include "mir/MIRModule.h"
#include "mir/MIRNode.h"
#include "mir/MIRValue.h"
#include "mir/MIRType.h"
#include "mir/MIRTerminator.h"
#include "semantic/SemanticAnalysis.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

class LLVMGen{
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    SemanticAnalysis& semanticAnalysis;
    llvm::Function* currentFunction;
public:
    LLVMGen(SemanticAnalysis& sema);
    llvm::Module* generate(MIRModule *module);
    llvm::Module* getModule();
    std::unique_ptr<llvm::Module> releaseModule();
    std::map<MIRValue*, llvm::Value*> valueMap;
    std::map<MIRBasicBlock*, llvm::BasicBlock*> blockMap;
private:
    // 各MIRノードのvisitメソッド
    void visit(MIRFunction *node);
    void visit(MIRBasicBlock *node);
    void visit(MIRInstruction *node);
    void visit(MIRAllocaInstruction *node);
    void visit(MIRStoreInstruction *node);
    void visit(MIRLoadInstruction *node);
    void visit(MIRUnaryInstruction *node);
    void visit(MIRBinaryInstruction *node);
    void visit(MIRTerminatorInstruction *node);
    void visit(MIRReturnInstruction *node);
    void visit(MIRBranchInstruction *node);
    void visit(MIRConditionBranchInstruction *node);
    void visit(MIRCallInstruction *node);
    llvm::Value* visit(MIRValue *node);
    llvm::Value* visit(MIRLiteralValue *node);
    llvm::Value* visit(MIRCastInstruction *node);
};
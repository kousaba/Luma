#include "mirgen/MIRGen.h" // インクルードパス変更
#include "ast/Definition.h"
#include "common/Global.h"
#include "mir/MIRInstruction.h"
#include "mir/MIRTerminator.h"
#include "mir/MIRValue.h"
#include <cassert>

MIRGen::MIRGen(SemanticAnalysis& sema) // クラス名変更
    : semanticAnalysis(sema), module(std::make_unique<MIRModule>("LumaMIRModule")) {}

std::unique_ptr<MIRModule> MIRGen::generate(std::shared_ptr<ProgramNode> root) { // クラス名変更
    visit(root.get());
    return std::move(module);
}

std::string MIRGen::newRegisterName() {
    return "%" + std::to_string(tempCounter++);
}

std::shared_ptr<MIRType> MIRGen::translateType(TypeNode* typeNode) {
    if (!typeNode) return std::make_shared<MIRType>(MIRType::TypeID::Void);

    std::string typeName = typeNode->getTypeName();
    if(typeName == "int") return std::make_shared<MIRType>(MIRType::TypeID::Int, "i64");
    if(typeName == "i32") return std::make_shared<MIRType>(MIRType::TypeID::Int, "i32");
    if(typeName == "float") return std::make_shared<MIRType>(MIRType::TypeID::Float, "double");
    if(typeName == "f32") return std::make_shared<MIRType>(MIRType::TypeID::Float, "float");
    if(typeName == "bool") return std::make_shared<MIRType>(MIRType::TypeID::Bool, "i1");
    if(typeName == "void") return std::make_shared<MIRType>(MIRType::TypeID::Void, "void");
    
    // TODO: char、struct、arrayなどの他の型

    errorHandler.errorReg("Unknown Type: " + typeName, 0);
    return std::make_shared<MIRType>(MIRType::TypeID::Unknown);
}

void MIRGen::setCurrentBlock(std::shared_ptr<MIRBasicBlock> block) { // クラス名変更
    currentBlock = block;
}

std::shared_ptr<MIRBasicBlock> MIRGen::createBasicBlock(const std::string& name) { // クラス名変更
    auto block = std::make_shared<MIRBasicBlock>(name);
    currentFunction->addBasicBlock(block);
    return block;
}


void MIRGen::visit(ProgramNode* node) { // クラス名変更
    auto mainFuncType = std::make_shared<MIRType>(MIRType::TypeID::Int, "i64");
    auto mainFunc = std::make_shared<MIRFunction>("main", mainFuncType);
    module->addFunction(mainFunc);
    currentFunction = mainFunc;
    auto entryBlock = createBasicBlock("entry");
    setCurrentBlock(entryBlock);
    for(auto& stmt : node->statements){
        if(dynamic_cast<FunctionDefNode*>(stmt.get())){
            continue;
        }
        visit(stmt.get());
    }
    if(!currentBlock->terminator){
        // "int" で型を引くのではなく、現在の関数の戻り値の型を直接使う
        auto mirReturnType = currentFunction->returnType; 
        
        if (!mirReturnType || mirReturnType->isVoid()) {
             // main関数はvoidであってはならないが、念のため
            errorHandler.errorReg("main function cannot be void.", 0);
        } else {
            currentBlock->setTerminator(std::make_shared<MIRReturnInstruction>(
                std::make_shared<MIRLiteralValue>(mirReturnType, "0")
            ));
        }
    }
    currentFunction = nullptr;
}

void MIRGen::visit(StatementNode* node) { // クラス名変更
    if (auto funcDef = dynamic_cast<FunctionDefNode*>(node)) return; // ProgramNodeで処理
    else if (auto varNode = dynamic_cast<VarDeclNode*>(node)) visit(varNode);
    else if (auto assignmentNode = dynamic_cast<AssignmentNode*>(node)) visit(assignmentNode);
    else if (auto ifNode = dynamic_cast<IfNode*>(node)) visit(ifNode);
    else if (auto forNode = dynamic_cast<ForNode*>(node)) visit(forNode);
    else if (auto exprStmtNode = dynamic_cast<ExprStatementNode*>(node)) visit(exprStmtNode);
    else if (auto returnNode = dynamic_cast<ReturnNode*>(node)) visit(returnNode);
    
    else 
        errorHandler.errorReg("Unknown Statement visited in MIRGen.", 0);
}

void MIRGen::visit(BlockNode* node) {
    for (const auto& stmt : node->statements) {
        visit(stmt.get());
    }
}

void MIRGen::visit(FunctionDefNode* node) {
    // auto returnType = translateType(node->returnType.get()); // 修正: returnTypeを使用
    // auto func = std::make_shared<MIRFunction>(node->name, returnType);
    // module->addFunction(func);
    
    // std::shared_ptr<MIRFunction> prevFunction = currentFunction;
    // currentFunction = func;

    // auto entryBlock = createBasicBlock(node->name + "_entry");
    // setCurrentBlock(entryBlock);

    // for (size_t i = 0; i < node->parameters.size(); ++i) {
    //     auto argType = translateType(semanticAnalysis.getType("int").get()); // 修正: getTypeを使用
    //     auto argValue = std::make_shared<MIRArgumentValue>(argType, node->parameters[i], i);
    //     func->arguments.push_back(argValue);

    //     auto allocaInst = std::make_shared<MIRAllocaInstruction>(
    //         argType, node->parameters[i], std::make_shared<MIRType>(MIRType::TypeID::Ptr, argType->name + "*"), newRegisterName()
    //     );
    //     entryBlock->addInstruction(allocaInst);

    //     auto storeInst = std::make_shared<MIRStoreInstruction>(argValue, allocaInst->result);
    //     entryBlock->addInstruction(storeInst);

    //     Symbol* symbol = semanticAnalysis.lookupSymbol(node->parameters[i]);
    //     if (symbol) {
    //         symbolValueMap[symbol] = allocaInst->result;
    //     }
    // }

    // visit(node->body.get());

    // if (!currentBlock->terminator) {
    //     currentBlock->setTerminator(std::make_shared<MIRReturnInstruction>(
    //         std::make_shared<MIRLiteralValue>(translateType(semanticAnalysis.getType("int").get()), "0")
    //     ));
    // }
    // currentFunction = prevFunction;
}

void MIRGen::visit(VarDeclNode* node) { 
    auto varMirType = translateType(node->type.get());
    if (!varMirType || varMirType->id == MIRType::TypeID::Unknown) {
        return;
    }

    auto entryBlock = currentFunction->basicBlocks.front();

    auto ptrType = std::make_shared<MIRType>(MIRType::TypeID::Ptr, varMirType->name + "*");
    auto allocaInst = std::make_shared<MIRAllocaInstruction>(
        varMirType, node->varName, ptrType, newRegisterName()
    );

    entryBlock->instructions.insert(entryBlock->instructions.begin(), allocaInst);

    Symbol* symbol = semanticAnalysis.lookupSymbol(node->varName);
    if (symbol) {
        symbolValueMap[symbol] = allocaInst->result;
    } else {
        errorHandler.errorReg("Symbol not found for variable: " + node->varName, 0);
        return;
    }

    if (node->initializer) {
        std::shared_ptr<MIRValue> initValue = visit(node->initializer.get());
        if (initValue) {
            auto storeInst = std::make_shared<MIRStoreInstruction>(initValue, allocaInst->result);
            currentBlock->addInstruction(storeInst);
        }
    }
}

void MIRGen::visit(AssignmentNode* node) {
    std::shared_ptr<MIRValue> val = visit(node->value.get());
    if (!val) {
        errorHandler.errorReg("Assignment of empty expression to variable '" + node->varName + "'", 0);
        return;
    }

    Symbol* symbol = semanticAnalysis.lookupSymbol(node->varName);
    if (!symbol || !symbolValueMap.count(symbol)) {
        errorHandler.errorReg("Assignment to undeclared variable '" + node->varName + "'", 0);
        return;
    }
    
    std::shared_ptr<MIRValue> varAddress = symbolValueMap[symbol];
    auto storeInst = std::make_shared<MIRStoreInstruction>(val, varAddress);
    currentBlock->addInstruction(storeInst);
}

void MIRGen::visit(IfNode* node) {
    std::shared_ptr<MIRValue> conditionValue = visit(node->condition.get());
    if (!conditionValue) {
        errorHandler.errorReg("The condition expression of if is an unknown expression.", 1);
        return;
    }

    std::shared_ptr<MIRBasicBlock> thenBlock = createBasicBlock("if.then");
    std::shared_ptr<MIRBasicBlock> elseBlock = createBasicBlock("if.else");
    std::shared_ptr<MIRBasicBlock> mergeBlock = createBasicBlock("if.merge");

    currentBlock->setTerminator(std::make_shared<MIRConditionBranchInstruction>(conditionValue, thenBlock, elseBlock));

    setCurrentBlock(thenBlock);
    visit(node->if_block.get());
    if (!currentBlock->terminator) {
        currentBlock->setTerminator(std::make_shared<MIRBranchInstruction>(mergeBlock));
    }

    setCurrentBlock(elseBlock);
    if (node->else_block) {
        visit(node->else_block.get());
    }
    if (!currentBlock->terminator) {
        currentBlock->setTerminator(std::make_shared<MIRBranchInstruction>(mergeBlock));
    }

    setCurrentBlock(mergeBlock);
}

void MIRGen::visit(ForNode* node) {
    std::shared_ptr<MIRBasicBlock> loopHeader = createBasicBlock("for.cond");
    std::shared_ptr<MIRBasicBlock> loopBody = createBasicBlock("for.body");
    std::shared_ptr<MIRBasicBlock> loopEnd = createBasicBlock("for.end");

    currentBlock->setTerminator(std::make_shared<MIRBranchInstruction>(loopHeader));

    setCurrentBlock(loopHeader);
    std::shared_ptr<MIRValue> conditionValue = visit(node->condition.get());
    if (!conditionValue) {
        errorHandler.errorReg("The condition expression of for is an unknown expression.", 1);
        return;
    }
    currentBlock->setTerminator(std::make_shared<MIRConditionBranchInstruction>(conditionValue, loopBody, loopEnd));

    setCurrentBlock(loopBody);
    visit(node->block.get());
    if (!currentBlock->terminator) {
        currentBlock->setTerminator(std::make_shared<MIRBranchInstruction>(loopHeader));
    }

    setCurrentBlock(loopEnd);
}

void MIRGen::visit(ReturnNode* node) {
    if (node->returnValue) {
        std::shared_ptr<MIRValue> retVal = visit(node->returnValue.get());
        currentBlock->setTerminator(std::make_shared<MIRReturnInstruction>(retVal));
    } else {
        currentBlock->setTerminator(std::make_shared<MIRReturnInstruction>());
    }
}

void MIRGen::visit(ExprStatementNode* node) {
    if (node->expression) {
        visit(node->expression.get());
    }
}

std::shared_ptr<MIRValue> MIRGen::visit(ExprNode* node) {
    if (auto cnode = dynamic_cast<NumberLiteralNode*>(node)) return visit(cnode);
    if (auto cnode = dynamic_cast<DecimalLiteralNode*>(node)) return visit(cnode);
    if (auto cnode = dynamic_cast<BinaryOpNode*>(node)) return visit(cnode);
    if (auto cnode = dynamic_cast<VariableRefNode*>(node)) return visit(cnode);
    if (auto cnode = dynamic_cast<FunctionCallNode*>(node)) return visit(cnode);
    if (auto cnode = dynamic_cast<CastNode*>(node)) return visit(cnode);
    
    errorHandler.errorReg("Unknown ExprNode visited in MIRGen.", 0);
    return nullptr;
}

std::shared_ptr<MIRValue> MIRGen::visit(NumberLiteralNode* node) {
    return std::make_shared<MIRLiteralValue>(translateType(node->type.get()), std::to_string(node->value));
}

std::shared_ptr<MIRValue> MIRGen::visit(DecimalLiteralNode* node) {
    return std::make_shared<MIRLiteralValue>(translateType(node->type.get()), std::to_string(node->value));
}

std::shared_ptr<MIRValue> MIRGen::visit(VariableRefNode* node) {
    Symbol* symbol = semanticAnalysis.lookupSymbol(node->name);
    if (!symbol || !symbolValueMap.count(symbol)) {
        errorHandler.errorReg("Undefined variable reference: " + node->name, 0);
        return nullptr;
    }
    std::shared_ptr<MIRValue> varAddress = symbolValueMap[symbol];
    auto loadInst = std::make_shared<MIRLoadInstruction>(varAddress, translateType(node->type.get()), newRegisterName());
    currentBlock->addInstruction(loadInst);
    return loadInst->result;
}

std::shared_ptr<MIRValue> MIRGen::visit(BinaryOpNode* node) {
    std::shared_ptr<MIRValue> lval = visit(node->left.get());
    std::shared_ptr<MIRValue> rval = visit(node->right.get());
    if (!lval || !rval) return nullptr;

    std::string op_str;
    if (node->op == "+") { op_str = "add"; }
    else if (node->op == "-") { op_str = "sub"; }
    else if (node->op == "*") { op_str = "mul"; }
    else if (node->op == "/") { op_str = "sdiv"; }
    else if (node->op == "==") { op_str = "icmp eq"; }
    else if (node->op == "!=") { op_str = "icmp ne"; }
    else if (node->op == "<") { op_str = "icmp slt"; }
    else if (node->op == ">") { op_str = "icmp sgt"; }
    else if (node->op == "<=") { op_str = "icmp sle"; }
    else if (node->op == ">=") { op_str = "icmp sge"; }
    else {
        errorHandler.errorReg("Unknown operator: " + node->op, 0);
        return nullptr;
    }

    auto binInst = std::make_shared<MIRBinaryInstruction>(op_str, lval, rval, translateType(node->type.get()), newRegisterName());
    currentBlock->addInstruction(binInst);
    return binInst->result;
}

std::shared_ptr<MIRValue> MIRGen::visit(FunctionCallNode* node) {
    std::vector<std::shared_ptr<MIRValue>> args;
    for (const auto& argExpr : node->args) {
        args.push_back(visit(argExpr.get()));
    }

    // PrintとInput
    if (node->calleeName == "print") {
        auto callInst = std::make_shared<MIRCallInstruction>("printf", args, std::make_shared<MIRType>(MIRType::TypeID::Int, "i32"), newRegisterName());
        currentBlock->addInstruction(callInst);
        return callInst->result;
    }
    if (node->calleeName == "input") {
        auto callInst = std::make_shared<MIRCallInstruction>("scanf", args, std::make_shared<MIRType>(MIRType::TypeID::Int, "i32"), newRegisterName());
        currentBlock->addInstruction(callInst);
        return callInst->result;
    }

    // 通常の関数呼び出し
    auto funcSymbol = semanticAnalysis.lookupSymbol(node->calleeName);
    if (!funcSymbol || funcSymbol->kind != SymbolKind::FUNC) {
        errorHandler.errorReg("Call to undefined function: " + node->calleeName, 0);
        return nullptr;
    }
    auto callInst = std::make_shared<MIRCallInstruction>(node->calleeName, args, translateType(node->type.get()), newRegisterName());
    currentBlock->addInstruction(callInst);
    return callInst->result;
}

std::shared_ptr<MIRValue> MIRGen::visit(CastNode* node) { // クラス名変更
    std::shared_ptr<MIRValue> operand = visit(node->expression.get());
    if (!operand) return nullptr;

    std::shared_ptr<MIRType> targetType = translateType(node->type.get());
    if (!targetType || targetType->id == MIRType::TypeID::Unknown) {
        errorHandler.errorReg("Invalid target type for cast.", 0);
        return nullptr;
    }

    // TODO: 正しいキャスト命令を決定する (例: sitofp、fptosi、intcast)
    auto castInst = std::make_shared<MIRCastInstruction>(operand, targetType, targetType, newRegisterName());
    currentBlock->addInstruction(castInst);
    return castInst->result;
}

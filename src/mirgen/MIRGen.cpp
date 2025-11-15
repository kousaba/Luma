#include "mirgen/MIRGen.h" // インクルードパス変更
#include "ast/Definition.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "common/Global.h"
#include "mir/MIRFunction.h"
#include "mir/MIRInstruction.h"
#include "mir/MIRTerminator.h"
#include "mir/MIRValue.h"
#include "types/TypeTranslate.h"
#include <cassert>
#include <memory>

MIRGen::MIRGen(SemanticAnalysis& sema) // クラス名変更
    : semanticAnalysis(sema), module(std::make_unique<MIRModule>("LumaMIRModule")) {}

std::unique_ptr<MIRModule> MIRGen::generate(std::shared_ptr<ProgramNode> root) { // クラス名変更
    visit(root.get());
    return std::move(module);
}

std::string MIRGen::newRegisterName() {
    return "%" + std::to_string(tempCounter++);
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
    // すべての関数定義を処理
    for(const auto& stmt : node->statements){
        if(auto funcDef = dynamic_cast<FunctionDefNode*>(stmt.get())){
            visit(funcDef);
        }
    }
    // main本体生成
    std::shared_ptr<MIRFunction> mainFunc = nullptr;
    for(const auto& func : module->functions){
        if(func->name == "main"){
            mainFunc = func;
            break;
        }
    }
    if(!mainFunc){
        auto mainFuncType = std::make_shared<MIRType>(MIRType::TypeID::Int, "i64");
        mainFunc = std::make_shared<MIRFunction>("main", mainFuncType);
        module->functions.insert(module->functions.begin(), mainFunc);
    }
    currentFunction = mainFunc;
    std::shared_ptr<MIRBasicBlock> entryBlock;
    if(mainFunc->basicBlocks.empty()){
        entryBlock = createBasicBlock("entry");
    }else{
        entryBlock = mainFunc->basicBlocks.front();
    }
    setCurrentBlock(entryBlock);
    // main関数スコープ内のグローバルな実行コードを処理
    for(const auto& stmt : node->statements){
        if(!dynamic_cast<FunctionDefNode*>(stmt.get())){
            visit(stmt.get());
        }
    }
    // main関数の末尾にreturnを追加
    if(!currentBlock->terminator){
        auto mirReturnType = currentFunction->returnType;
        if(!mirReturnType || mirReturnType->isVoid()){
            errorHandler.errorReg("main function must return an integer.", 0);
        }else{
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
    else if (auto arrayNode = dynamic_cast<ArrayDeclNode*>(node)) visit(arrayNode);
    else 
        errorHandler.errorReg("Unknown Statement visited in MIRGen.", 0);
}

void MIRGen::visit(BlockNode* node) {
    for (const auto& stmt : node->statements) {
        visit(stmt.get());
    }
}

void MIRGen::visit(FunctionDefNode* node) {
    // コンテキスト保存
    auto prevFunction = currentFunction;
    auto prevBlock = currentBlock;
    // MIRFunction作成
    auto returnMirType = TypeTranslate::toMirType(node->returnType.get());
    auto func = std::make_shared<MIRFunction>(node->name, returnMirType);
    module->addFunction(func);
    currentFunction = func;
    // エントリーブロック作成
    auto entryBlock = createBasicBlock("entry");
    setCurrentBlock(entryBlock);
    // 関数シンボル取得
    auto funcSymbol = std::dynamic_pointer_cast<FuncSymbol>(node->symbol);
    if(!funcSymbol){
        errorHandler.errorReg("Function symbol not found for " + node->name, 0);
        return;
    }
    // 引数処理
    assert(node->args.size() == funcSymbol->parameters.size());
    for(size_t i = 0;i < node->args.size();i++){
        const auto& argName = node->args[i];
        const auto& argTypeNode = node->argTypes[i];
        auto paramSymbol = funcSymbol->parameters[i];
        auto argMirType = TypeTranslate::toMirType(argTypeNode.get());
        auto mirArgument = std::make_shared<MIRArgumentValue>(argMirType, "%" + argName, i);
        currentFunction->addArgument(mirArgument);
        auto ptrType = std::make_shared<MIRType>(MIRType::TypeID::Ptr, argMirType->name + "*");
        auto allocaInst = std::make_shared<MIRAllocaInstruction>(
            argMirType, argName, ptrType, newRegisterName()
        );
        entryBlock->instructions.insert(entryBlock->instructions.begin(), allocaInst);
        symbolValueMap[paramSymbol] = allocaInst->result;
        auto storeInst = std::make_shared<MIRStoreInstruction>(mirArgument, allocaInst->result);
        currentBlock->addInstruction(storeInst);
    }
    // 関数本体のコード作成
    visit(node->body.get());
    // return処理
    if(!currentBlock->terminator){
        if(currentFunction->returnType->isVoid()){
            currentBlock->setTerminator(std::make_shared<MIRReturnInstruction>());
        }else{
            errorHandler.errorReg("Function '" + node->name + "' has non-void return type but no return statement.", 0);
            auto defaultValue = std::make_shared<MIRLiteralValue>(currentFunction->returnType, "0");
            currentBlock->setTerminator(std::make_shared<MIRReturnInstruction>(defaultValue));
            return;
        }
    }
    // コンテキスト復元
    currentFunction = prevFunction;
    currentBlock = prevBlock;
}

void MIRGen::visit(VarDeclNode* node) { 
    auto varMirType = TypeTranslate::toMirType(node->type.get());
    if (!varMirType || varMirType->id == MIRType::TypeID::Unknown) {
        return;
    }

    auto entryBlock = currentFunction->basicBlocks.front();

    auto ptrType = std::make_shared<MIRType>(MIRType::TypeID::Ptr, varMirType->name + "*");
    auto allocaInst = std::make_shared<MIRAllocaInstruction>(
        varMirType, node->varName, ptrType, newRegisterName()
    );

    entryBlock->instructions.insert(entryBlock->instructions.begin(), allocaInst);

    auto symbol = node->symbol;
    if (symbol) {
        symbolValueMap[symbol] = allocaInst->result;
    } else {
        errorHandler.errorReg("Symbol not attached to VarDeclNode for: " + node->varName, 0);
        return;
    }

    if (node->initializer) {
        if (auto arrayLit = dynamic_cast<ArrayLiteralNode*>(node->initializer.get())) {
            // 配列リテラルによる初期化
            for (size_t i = 0; i < arrayLit->elem.size(); ++i) {
                auto indexValue = std::make_shared<MIRLiteralValue>(std::make_shared<MIRType>(MIRType::TypeID::Int, "int"), std::to_string(i));
                auto gepInst = std::make_shared<MIRGepInstruction>(
                    allocaInst->result,
                    indexValue,
                    TypeTranslate::toMirType(arrayLit->elem[i]->type.get()),
                    varMirType,
                    newRegisterName()
                );
                currentBlock->addInstruction(gepInst);
                
                std::shared_ptr<MIRValue> elementValue = visit(arrayLit->elem[i].get());
                auto storeInst = std::make_shared<MIRStoreInstruction>(elementValue, gepInst->result);
                currentBlock->addInstruction(storeInst);
            }
        } else {
            // 通常の式による初期化
            std::shared_ptr<MIRValue> initValue = visit(node->initializer.get());
            if (initValue) {
                auto storeInst = std::make_shared<MIRStoreInstruction>(initValue, allocaInst->result);
                currentBlock->addInstruction(storeInst);
            }
        }
    }
}

void MIRGen::visit(ArrayDeclNode *node){
    auto arrMirType = TypeTranslate::toMirType(node->type.get());
    if(!arrMirType || arrMirType->id == MIRType::TypeID::Unknown){
        return;
    }
    auto entryBlock = currentFunction->basicBlocks.front();
    auto ptrType = std::make_shared<MIRType>(MIRType::TypeID::Ptr, arrMirType->name + "*");
    auto allocaInst = std::make_shared<MIRAllocaInstruction>(
        arrMirType, node->arrayName, ptrType, newRegisterName(), node->size
    );

    entryBlock->instructions.insert(entryBlock->instructions.begin(), allocaInst);
    
    auto symbol = node->symbol;
    if(symbol){
        symbolValueMap[symbol] = allocaInst->result;
    }else{
        errorHandler.errorReg("Symbol not attached to ArrayDeclNode for: " + node->arrayName, 0);
        return;
    }
    // TODO: 初期化式(配列リテラルを作ってから)
}

void MIRGen::visit(AssignmentNode* node) {
    std::shared_ptr<MIRValue> val = visit(node->value.get());
    if (!val) {
        errorHandler.errorReg("Assignment of empty expression to variable '" + node->varName + "'", 0);
        return;
    }

    auto symbol = node->symbol;
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
    node->thenBlock = thenBlock.get();
    node->elseBlock = elseBlock.get();
    node->mergeBlock = mergeBlock.get();

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
    if (auto cnode = dynamic_cast<ArrayRefNode*>(node)) return visit(cnode); // 追加
    
    errorHandler.errorReg("Unknown ExprNode visited in MIRGen.", 0);
    return nullptr;
}

std::shared_ptr<MIRValue> MIRGen::visit(NumberLiteralNode* node) {
    return std::make_shared<MIRLiteralValue>(TypeTranslate::toMirType(node->type.get()), std::to_string(node->value));
}

std::shared_ptr<MIRValue> MIRGen::visit(DecimalLiteralNode* node) {
    return std::make_shared<MIRLiteralValue>(TypeTranslate::toMirType(node->type.get()), std::to_string(node->value));
}

std::shared_ptr<MIRValue> MIRGen::visit(ArrayLiteralNode *node){
    auto arrayType = TypeTranslate::toMirType(node->type.get());
    auto elementType = TypeTranslate::toMirType(node->elem[0]->type.get());
    size_t arraySize = node->elem.size();
    // スタックに確保
    auto arrayPtrType = std::make_shared<MIRType>(arrayType);
    auto allocaInst = std::make_shared<MIRAllocaInstruction>(
        arrayType, "arrayLit", arrayPtrType, newRegisterName(), arraySize
    );
    currentBlock->addInstruction(allocaInst);
    std::shared_ptr<MIRValue> arrayPtr = allocaInst->result;
    for(size_t i = 0;i < node->elem.size();i++){
        auto indexValue = std::make_shared<MIRLiteralValue>(std::make_shared<MIRType>(MIRType::TypeID::Int, "int"), std::to_string(i));
        auto gepInst = std::make_shared<MIRGepInstruction>(
            arrayPtr,
            indexValue,
            elementType,
            arrayType,
            newRegisterName()
        );
        currentBlock->addInstruction(gepInst);
        std::shared_ptr<MIRValue> elementValue = visit(node->elem[i].get());
        auto storeInst = std::make_shared<MIRStoreInstruction>(elementValue, gepInst->result);
        currentBlock->addInstruction(storeInst);
    }
    return arrayPtr;
}

std::shared_ptr<MIRValue> MIRGen::visit(VariableRefNode* node) {
    auto symbol = node->symbol;
    if (!symbol || !symbolValueMap.count(symbol)) {
        errorHandler.errorReg("Undefined variable reference: " + node->name, 0);
        return nullptr;
    }
    std::shared_ptr<MIRValue> varAddress = symbolValueMap[symbol];
    auto loadInst = std::make_shared<MIRLoadInstruction>(varAddress, TypeTranslate::toMirType(node->type.get()), newRegisterName());
    currentBlock->addInstruction(loadInst);
    return loadInst->result;
}

std::shared_ptr<MIRValue> MIRGen::visit(ArrayRefNode *node){
    auto symbol = node->symbol;
    if (!symbol || !symbolValueMap.count(symbol)) {
        errorHandler.errorReg("Undefined variable reference: " + node->name, 0);
        return nullptr;
    }
    std::shared_ptr<MIRValue> arrayAddress = symbolValueMap[symbol];
    std::shared_ptr<MIRValue> indexValue = nullptr;
    if(auto* numLit = dynamic_cast<NumberLiteralNode*>(node->idx.get())){
        indexValue = visit(numLit);
    }else{
        indexValue = visit(node->idx.get());
    }
    if(!indexValue){
        errorHandler.errorReg("Invalid array index expression.", 0);
        return nullptr;
    }
    auto elementType = TypeTranslate::toMirType(node->type.get());
    // arrayAddress の型は int[5]*
    // ここから int[5] を抽出する
    std::shared_ptr<MIRType> arrayPtrType = arrayAddress->type; // int[5]*
    std::string arrayPtrTypeName = arrayPtrType->name; // "int[5]*"
    size_t starPos = arrayPtrTypeName.rfind('*');
    std::string arrayTypeName = arrayPtrTypeName.substr(0, starPos); // "int[5]"
    // arrayTypeName は "int[5]" の形式
    // ここから要素型 "int" とサイズ "5" をパースする
    size_t openBracketPos = arrayTypeName.find('[');
    size_t closeBracketPos = arrayTypeName.find(']');
    std::string elementTypeName = arrayTypeName.substr(0, openBracketPos); // "int"
    std::string arraySizeStr = arrayTypeName.substr(openBracketPos + 1, closeBracketPos - (openBracketPos + 1)); // "5"
    size_t arraySize = std::stoul(arraySizeStr); // 5

    std::shared_ptr<MIRType> arrayElementType = std::make_shared<MIRType>(getMirTypeIDFromString(elementTypeName), elementTypeName); // int
    std::shared_ptr<MIRType> ptrOrArrayType = std::make_shared<MIRType>(arrayElementType, arraySize); // int[5]

    auto gepInst = std::make_shared<MIRGepInstruction>(
        arrayAddress,
        indexValue,
        elementType, // int
        ptrOrArrayType, // int[5]
        newRegisterName()
    );
    currentBlock->addInstruction(gepInst);
    auto loadInst = std::make_shared<MIRLoadInstruction>(
        gepInst->result,
        elementType,
        newRegisterName()
    );
    currentBlock->addInstruction(loadInst);
    return loadInst->result;
}



std::shared_ptr<MIRValue> MIRGen::visit(BinaryOpNode* node) {
    std::shared_ptr<MIRValue> lval = visit(node->left.get());
    std::shared_ptr<MIRValue> rval = visit(node->right.get());
    if (!lval || !rval) return nullptr;

    std::string op_str;
    bool isFloat = lval->type->isFloat();

    if (node->op == "+") { op_str = isFloat ? "fadd" : "add"; }
    else if (node->op == "-") { op_str = isFloat ? "fsub" : "sub"; }
    else if (node->op == "*") { op_str = isFloat ? "fmul" : "mul"; }
    else if (node->op == "/") { op_str = isFloat ? "fdiv" : "sdiv"; }
    else if (node->op == "==") { op_str = isFloat ? "fcmp eq" : "icmp eq"; }
    else if (node->op == "!=") { op_str = isFloat ? "fcmp ne" : "icmp ne"; }
    else if (node->op == "<") { op_str = isFloat ? "fcmp lt" : "icmp lt"; }
    else if (node->op == ">") { op_str = isFloat ? "fcmp gt" : "icmp gt"; }
    else if (node->op == "<=") { op_str = isFloat ? "fcmp le" : "icmp le"; }
    else if (node->op == ">=") { op_str = isFloat ? "fcmp ge" : "icmp ge"; }
    else {
        errorHandler.errorReg("Unknown operator: " + node->op, 0);
        return nullptr;
    }

    auto binInst = std::make_shared<MIRBinaryInstruction>(op_str, lval, rval, TypeTranslate::toMirType(node->type.get()), newRegisterName());
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
    auto funcSymbol = std::dynamic_pointer_cast<FuncSymbol>(node->symbol);
    if (!funcSymbol) {
        errorHandler.errorReg("Call to undefined function: " + node->calleeName, 0);
        return nullptr;
    }
    auto callInst = std::make_shared<MIRCallInstruction>(node->calleeName, args, TypeTranslate::toMirType(node->type.get()), newRegisterName());
    currentBlock->addInstruction(callInst);
    return callInst->result;
}

std::shared_ptr<MIRValue> MIRGen::visit(CastNode* node) { // クラス名変更
    std::shared_ptr<MIRValue> operand = visit(node->expression.get());
    if (!operand) return nullptr;

    std::shared_ptr<MIRType> targetType = TypeTranslate::toMirType(node->type.get());
    if (!targetType || targetType->id == MIRType::TypeID::Unknown) {
        errorHandler.errorReg("Invalid target type for cast.", 0);
        return nullptr;
    }

    auto sourceType = operand->type;
    CastOpcode castOp;

    if (sourceType->isInteger() && targetType->isFloat()) {
        castOp = CastOpcode::SIToFP;
    } else if (sourceType->isFloat() && targetType->isInteger()) {
        castOp = CastOpcode::FPToSI;
    } else if (sourceType->isInteger() && targetType->isInteger()) {
        castOp = CastOpcode::IntCast;
    } else if (sourceType->isFloat() && targetType->isFloat()) {
        castOp = CastOpcode::FPCast;
    } else if (sourceType->isPointer() && targetType->isInteger()) {
        castOp = CastOpcode::PtrToInt;
    } else if (sourceType->isInteger() && targetType->isPointer()) {
        castOp = CastOpcode::IntToPtr;
    } else if (sourceType->isPointer() && targetType->isPointer()) {
        castOp = CastOpcode::PtrCast;
    } else {
        errorHandler.errorReg("Unsupported cast operation in MIRGen.", 0);
        return nullptr;
    }

    auto castInst = std::make_shared<MIRCastInstruction>(castOp, operand, targetType, newRegisterName());
    currentBlock->addInstruction(castInst);
    return castInst->result;
}

MIRType::TypeID MIRGen::getMirTypeIDFromString(const std::string& typeName){
    if (typeName == "int") return MIRType::TypeID::Int;
    if (typeName == "float") return MIRType::TypeID::Float;
    if (typeName == "void") return MIRType::TypeID::Void;
    if (typeName == "bool") return MIRType::TypeID::Bool;
    // 他の型も必要に応じて追加
    errorHandler.errorReg("Unknown MIRTypeID for string: " + typeName, 0);
    return MIRType::TypeID::Void; // エラー時のデフォルト値
}

#include "SemanticAnalysis.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "common/ErrorDef.h"
#include "common/ErrorHandler.h"
#include "common/Global.h"
#include "types/Type.h"
#include <memory>
#include <string>


// コンストラクタ
SemanticAnalysis::SemanticAnalysis(){
    // 基本型のポインタを持つ
    typePtr["int"] = std::make_shared<BasicTypeNode>("int");
    typePtr["float"] = std::make_shared<BasicTypeNode>("float");
    typePtr["i32"] = std::make_shared<BasicTypeNode>("i32");
    typePtr["f32"] = std::make_shared<BasicTypeNode>("f32");
    typePtr["char"] = std::make_shared<BasicTypeNode>("char");
    typePtr["bool"] = std::make_shared<BasicTypeNode>("bool");
    typePtr["void"] = std::make_shared<BasicTypeNode>("void");
}

// 有効な型か判別するヘルパー関数
bool SemanticAnalysis::is_type(std::string typeName){
    bool isBasicType = (typeName == "int" || typeName == "i32" || typeName == "void" || typeName == "float" || 
                        typeName == "f32" || typeName == "char");
    return isBasicType;
}

// main用
// エラーがあるか
bool SemanticAnalysis::hasErrors(){
    return errorHandler.hasError();
}

void SemanticAnalysis::enterScope(){
    symbolTable.push_back(std::map<std::string, std::unique_ptr<Symbol>>());
}
void SemanticAnalysis::leaveScope(){
    if(!symbolTable.empty()){
        symbolTable.pop_back();
    }else{
        errorHandler.compilerErrorReg(CompilerErrorCode::LEAVESCOPE_WITH_EMPTY_SYMBOLTABLE, {});
    }
}
bool SemanticAnalysis::addSymbol(std::unique_ptr<Symbol> symbol){
    if(symbolTable.empty()){
        errorHandler.compilerErrorReg(CompilerErrorCode::ADDSYMBOL_WITH_NO_SCOPE, {});
        return false;
    }
    if(symbolTable.back().contains(symbol->name)){
        errorHandler.errorReg(ErrorCode::ADDSYMBOL_ALREADY_CONTAINS, {});
        return false;
    }
    symbolTable.back()[symbol->name] = std::move(symbol);
    return true;
}
Symbol* SemanticAnalysis::lookupSymbol(const std::string& name){
    for(auto it = symbolTable.rbegin(); it != symbolTable.rend(); ++it){
        auto symbolIt = it->find(name);
        if(symbolIt != it->end()){
            return symbolIt->second.get();
        }
    }
    return nullptr; // 見つからなかった
}

TypeNode* SemanticAnalysis::visit(ExprNode *node){
    if(auto cnode = dynamic_cast<BinaryOpNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<FunctionCallNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<NumberLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<DecimalLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<CastNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VariableRefNode*>(node)) return visit(cnode);
    // TODO: 続き
    else{
        // errorHandler.errorReg("SemanticAnalysis::visit(ExprNode *node) failed to find correct type.", -1);
        errorHandler.compilerErrorReg(CompilerErrorCode::EXPR_VISIT_COULDNOT_CAST, {std::string(typeid(*node).name())});
        return nullptr;
    }
}

void SemanticAnalysis::visit(VarDeclNode *node){
    std::shared_ptr<TypeNode> varType = nullptr;
    TypeNode* inferredTypeRaw = nullptr;

    // ステップ1: 変数の型を決定する
    if (node->type) {
        // 型注釈がある場合
        varType = node->type;
    } else if (node->initializer) {
        // 型注釈がなく、初期化式がある場合 -> 型推論
        inferredTypeRaw = visit(node->initializer.get());
        if (inferredTypeRaw) {
            // 推論された型 (TypeNode*) から shared_ptr を安全に作成し、
            // varType と node->type の両方に設定する
            varType = inferredTypeRaw->shared_from_this();
            node->type = varType;
        }
    } else {
        errorHandler.errorReg(ErrorCode::VARDECL_NO_TYPE_AND_INIT, {node->varName});
        return;
    }

    if (!varType) {
        errorHandler.errorReg(ErrorCode::VARDECL_CANNOT_DETERMINE_TYPE, {node->varName});
        return;
    }

    // ステップ2: 決定した型が有効かチェックする
    std::string typeName = varType->getTypeName();
    if (typeName == "void") {
        errorHandler.errorReg(ErrorCode::VARDECL_CANNOT_DECLARE_VOID, {node->varName});
        return;
    }
    if (!is_type(typeName)) {
        Symbol* typeSymbol = lookupSymbol(typeName);
        if (!typeSymbol || (typeSymbol->kind != SymbolKind::STRUCT)) {
            errorHandler.errorReg(ErrorCode::VARDECL_TYPE_NOT_DEFINED, {typeName, node->varName});
            return;
        }
    }

    // ステップ3: シンボルテーブルに登録する
    auto newSymbol = std::make_unique<Symbol>(node->varName, varType.get());
    if (!addSymbol(std::move(newSymbol))) {
        // addSymbol内で二重定義エラーが出力される
        return;
    }

    // ステップ4: 型注釈と初期化式の型が一致するかチェックする
    if (node->type && node->initializer) {
        TypeNode* initializerType = visit(node->initializer.get());
        if (initializerType && initializerType->getTypeName() != node->type->getTypeName()) {
            errorHandler.errorReg(ErrorCode::VARDECL_INIT_TYPE_MISMATCH, {node->varName, node->type->getTypeName(), initializerType->getTypeName()});
        }
    }
}

TypeNode* SemanticAnalysis::visit(BinaryOpNode *node){
    TypeNode* leftType = visit(node->left.get());
    TypeNode* rightType = visit(node->right.get());
    if(!leftType || !rightType){
        return nullptr;
    }
    if(leftType->getTypeName() != rightType->getTypeName()){
        errorHandler.errorReg(ErrorCode::BINARYOP_OPERAND_MISMATCH, {node->op, leftType->getTypeName(), rightType->getTypeName()});
        return nullptr;
    }
    
    // 比較演算子はboolを返す
    if(node->op == "==" || node->op == "!=" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">="){
        node->type = typePtr["bool"]->shared_from_this();
        return typePtr["bool"].get();
    }

    node->type = leftType->shared_from_this();
    return leftType;
}

TypeNode* SemanticAnalysis::visit(FunctionCallNode *node){
    // 引数の各式を先にvisitして、型情報をASTに付与する
    for(const auto& arg : node->args){
        visit(arg.get());
    }

    // 組み込み関数の特別扱い
    if(node->calleeName == "print" || node->calleeName == "input"){
        // printとinputは文として扱われ、値を持たない
        node->type = typePtr["void"]->shared_from_this();
        return typePtr["void"].get();
    }

    Symbol* funcSymbol = lookupSymbol(node->calleeName);
    if(!funcSymbol){
        errorHandler.errorReg(ErrorCode::FUNCCALL_NOT_DEFINED, {node->calleeName});
        return nullptr;
    }
    
    if(funcSymbol->kind != SymbolKind::FUNC){
        errorHandler.errorReg(ErrorCode::FUNCCALL_NOT_FUNC_CALL, {node->calleeName});
        return nullptr;
    }

    const auto& expectedArgTypes = funcSymbol->argTypes;
    const auto& actualArgs = node->args;
    if(actualArgs.size() != expectedArgTypes.size()){
        errorHandler.errorReg(ErrorCode::FUNCCALL_ARG_SIZE_MISMATCH, {node->calleeName, std::to_string(expectedArgTypes.size()), std::to_string(actualArgs.size())});
        return nullptr;
    }

    for(size_t i = 0; i < actualArgs.size(); i++){
        // 上でvisit済みなので、型を取得するだけ
        TypeNode* actualType = actualArgs[i]->type.get();
        if(!actualType) return nullptr; // 引数の式の中でエラー

        TypeNode* expectedType = expectedArgTypes[i];
        if(actualType->getTypeName() != expectedType->getTypeName()){
            errorHandler.errorReg(ErrorCode::FUNCCALL_ARG_TYPE_MISMATCH, {std::to_string(i + 1), node->calleeName, expectedType->getTypeName(), actualType->getTypeName()});
            return nullptr;
        }
    }

    node->type = funcSymbol->type->shared_from_this();
    return funcSymbol->type;
}

void SemanticAnalysis::visit(AssignmentNode *node){
    Symbol* varSymbol = lookupSymbol(node->varName);
    if(!varSymbol){
        errorHandler.errorReg(ErrorCode::ASSIGNMENT_NOT_DEFINED, {node->varName});
        return;
    }
    if(varSymbol->kind != SymbolKind::VAR){
        errorHandler.errorReg(ErrorCode::ASSIGNMENT_NOT_VARIABLE, {node->varName});
        return;
    }
    TypeNode* varType = varSymbol->type;
    TypeNode* valueType = visit(node->value.get());
    if(!valueType){
        // visitしたときにエラーメッセージが出ているはず
        return;
    }
    if(varType->getTypeName() != valueType->getTypeName()){
        errorHandler.errorReg(ErrorCode::ASSIGNMENT_TYPE_MISMATCH, {valueType->getTypeName(), node->varName, varType->getTypeName()});
        return;
    }
    return;
}

TypeNode* SemanticAnalysis::visit(NumberLiteralNode *node){
    node->type = typePtr["int"]->shared_from_this();
    return typePtr["int"].get();
}
TypeNode* SemanticAnalysis::visit(DecimalLiteralNode *node){
    node->type = typePtr["float"]->shared_from_this();
    return typePtr["float"].get();
}
TypeNode* SemanticAnalysis::visit(VariableRefNode *node){
    Symbol* varSymbol = lookupSymbol(node->name);
    if(!varSymbol){
        errorHandler.errorReg(ErrorCode::VARREF_NOT_DEFINED, {node->name});
        return nullptr;
    }
    if(varSymbol->kind != SymbolKind::VAR){
        errorHandler.errorReg(ErrorCode::VARREF_NOT_VARIABLE, {node->name});
        return nullptr;
    }
    node->type = varSymbol->type->shared_from_this();
    return varSymbol->type;
}
TypeNode* SemanticAnalysis::visit(CastNode *node){
    TypeNode* exprType = visit(node->expression.get());
    if(!exprType){
        // visitするときにエラーが出ているはず
        return nullptr;
    }
    if(!node->type){
        errorHandler.compilerErrorReg(CompilerErrorCode::CAST_NODE_TYPE_NULL, {});
        return nullptr;
    }
    TypeNode* type = node->type.get();
    std::string typeName;
    if(auto cnode = dynamic_cast<BasicTypeNode*>(type)) typeName = cnode->getTypeName();
    else{
        errorHandler.errorReg(ErrorCode::CAST_TO_NON_BASIC, {});
    }
    if(!is_type(typeName)){
        errorHandler.errorReg(ErrorCode::CAST_INVALID_TYPE, {});
        return nullptr;
    }
    // TODO: 有効なキャストか確かめる処理
    return node->type.get();
}

// 式以外
void SemanticAnalysis::visit(ProgramNode *node){
    enterScope();
    currentFunctionReturnType = typePtr["int"].get();
    for(auto stmt : node->statements){
        visit(stmt.get());
    }
    currentFunctionReturnType = nullptr;
    leaveScope();
}
void SemanticAnalysis::visit(BlockNode *node){
    enterScope();
    for(auto stmt : node->statements){
        visit(stmt.get());
    }
    leaveScope();
}
void SemanticAnalysis::visit(IfNode *node){
    TypeNode* condType = visit(node->condition.get());
    if(condType->getTypeName() != "bool"){
        errorHandler.errorReg(ErrorCode::IF_NOT_BOOL, {condType->getTypeName()});
        return;
    }
    visit(node->if_block.get());
    if(node->else_block){
        visit(node->else_block.get());
    }
}
void SemanticAnalysis::visit(ForNode *node){
    TypeNode* condType = visit(node->condition.get());
    if(condType->getTypeName() != "bool"){
        errorHandler.errorReg(ErrorCode::FOR_NOT_BOOL, {condType->getTypeName()});
        return;
    }
    visit(node->block.get());
}
void SemanticAnalysis::visit(ReturnNode *node){
    if(!currentFunctionReturnType){
        errorHandler.errorReg("Return statement outside of a function.", 0);
        return;
    }
    if(node->returnValue){
        TypeNode* returnExprType = visit(node->returnValue.get());
        if(returnExprType){
            if (returnExprType->getTypeName() == "unknown" || 
            (dynamic_cast<NumberLiteralNode*>(node->returnValue.get()) && returnExprType->getTypeName()== "int")){
                node->returnValue->type = currentFunctionReturnType->shared_from_this();
                returnExprType = currentFunctionReturnType;
            }
            if(returnExprType->getTypeName() != currentFunctionReturnType->getTypeName()){
                errorHandler.errorReg("Return type mismatch. Expected '" + currentFunctionReturnType->getTypeName() 
                + "' but got '" + returnExprType->getTypeName() + "'.", 0);
            }
        }
    }else{
        if(currentFunctionReturnType->getTypeName() != "void"){
            errorHandler.errorReg("Return value expected for function returning '" + currentFunctionReturnType->getTypeName()
            + "'.", 0);
        }
    }
}
void SemanticAnalysis::visit(ExprStatementNode *node){
    if(node->expression){
        visit(node->expression.get());
    }else{
        errorHandler.warnReg(WarnCode::EXPR_STMT_NO_EXPR, {});
    }
}

void SemanticAnalysis::visit(StatementNode *node){
    if(auto cnode = dynamic_cast<BlockNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<IfNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ForNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ReturnNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VarDeclNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<AssignmentNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ExprStatementNode*>(node)) return visit(cnode);
    else{
        errorHandler.compilerErrorReg(CompilerErrorCode::STMT_VISIT_COULDNOT_CAST, {std::string(typeid(*node).name())});
    }
}

std::shared_ptr<TypeNode> SemanticAnalysis::getType(const std::string& name) const {
    auto it = typePtr.find(name);
    if (it != typePtr.end()) {
        return it->second;
    }
    return nullptr;
}
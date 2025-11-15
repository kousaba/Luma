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

void SemanticAnalysis::analyze(std::shared_ptr<ProgramNode> root){
    globalScope = std::make_shared<Scope>();
    currentScope = globalScope;
    currentFunction = nullptr;
    visit(root.get());
}

void SemanticAnalysis::enterScope(){
    currentScope = std::make_shared<Scope>(currentScope);
}
void SemanticAnalysis::leaveScope(){
    if(currentScope->parent){
        currentScope = currentScope->parent;
    }else{
        errorHandler.compilerErrorReg(CompilerErrorCode::LEAVESCOPE_WITH_EMPTY_SYMBOLTABLE, {});
    }
}
std::shared_ptr<TypeNode> SemanticAnalysis::visit(ExprNode *node){
    if(auto cnode = dynamic_cast<BinaryOpNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<FunctionCallNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<NumberLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<DecimalLiteralNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<CastNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VariableRefNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ArrayRefNode*>(node)) return visit(cnode);
    else{
        // errorHandler.errorReg("SemanticAnalysis::visit(ExprNode *node) failed to find correct type.", -1);
        errorHandler.compilerErrorReg(CompilerErrorCode::EXPR_VISIT_COULDNOT_CAST, {std::string(typeid(*node).name())});
        return nullptr;
    }
}

void SemanticAnalysis::visit(VarDeclNode *node){
    if(currentScope->lookupCurrent(node->varName)){
        errorHandler.errorReg("Variable '" + node->varName + "' already defined in this scope.", 0);
        return;
    }
    auto varType = node->type; 

    if(node->initializer){
        auto initType = visit(node->initializer.get());
        if (!initType) {
            errorHandler.errorReg("Cannot determine type of initializer for variable '" + node->varName + "'.", 0);
            return;
        }

        if (!varType) {
            varType = initType;
            node->type = initType;
        } else if (varType->getTypeName() != initType->getTypeName()) {
            errorHandler.errorReg("Initializer type mismatch for variable '" + node->varName + "'.", 0);
        }
    }

    if (!varType) {
        errorHandler.errorReg("Variable '" + node->varName + "' has no type and no initializer.", 0);
        return;
    }

    auto varSymbol = std::make_shared<VarSymbol>(node->varName, varType, currentScope);
    currentScope->define(varSymbol);
    node->symbol = varSymbol;
}

void SemanticAnalysis::visit(ArrayDeclNode *node){
    if(currentScope->lookupCurrent(node->arrayName)){
        errorHandler.errorReg("Array '" + node->arrayName + "' already defined in this scope.", 0);
        return;
    }
    auto arrayType = node->type;
    // TODO: 初期化式
    auto arraySymbol = std::make_shared<ArraySymbol>(node->arrayName, arrayType, currentScope);
    currentScope->define(arraySymbol);
    node->symbol = arraySymbol;
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(BinaryOpNode *node){
    std::shared_ptr<TypeNode> leftType = visit(node->left.get());
    std::shared_ptr<TypeNode> rightType = visit(node->right.get());
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
        return typePtr["bool"];
    }

    node->type = leftType->shared_from_this();
    return leftType;
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(FunctionCallNode *node){
    auto symbol = currentScope->lookup(node->calleeName);
    if(node->calleeName == "input" || node->calleeName == "print"){
        return nullptr;
    }
    if(!symbol){
        errorHandler.errorReg("Function '" + node->calleeName + "' not defined.", 0);
        return nullptr;
    }
    if(symbol->kind != SymbolKind::FUNC){
        errorHandler.errorReg("'" + node->calleeName + "' is not a function.", 0);
        return nullptr;
    }
    auto funcSymbol = std::static_pointer_cast<FuncSymbol>(symbol);
    if(node->args.size() != funcSymbol->parameters.size()){
        errorHandler.errorReg("Incorrect number of arguments for function '" + node->calleeName + "'.", 0);
    }
    for(size_t i = 0;i < node->args.size() && i < funcSymbol->parameters.size();i++){
        auto argType = visit(node->args[i].get());
        auto paramType = funcSymbol->parameters[i]->type;
        if(argType && paramType && argType.get()->getTypeName() != paramType.get()->getTypeName()){
            errorHandler.errorReg("Type mismatch for argument " + std::to_string(i + 1) + " of function '"
            + node->calleeName + "'.", 0);
        }
    }
    node->symbol = funcSymbol;
    return funcSymbol->type;
}

void SemanticAnalysis::visit(AssignmentNode *node){
    auto varSymbol = currentScope->lookup(node->varName);
    if(!varSymbol){
        errorHandler.errorReg(ErrorCode::ASSIGNMENT_NOT_DEFINED, {node->varName});
        return;
    }
    if(varSymbol->kind != SymbolKind::VAR){
        errorHandler.errorReg(ErrorCode::ASSIGNMENT_NOT_VARIABLE, {node->varName});
        return;
    }
    node->symbol = varSymbol;
    std::shared_ptr<TypeNode> varType = varSymbol->type;
    std::shared_ptr<TypeNode> valueType = visit(node->value.get());
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

std::shared_ptr<TypeNode> SemanticAnalysis::visit(NumberLiteralNode *node){
    node->type = typePtr["int"]->shared_from_this();
    return typePtr["int"];
}
std::shared_ptr<TypeNode> SemanticAnalysis::visit(DecimalLiteralNode *node){
    node->type = typePtr["float"]->shared_from_this();
    return typePtr["float"];
}
std::shared_ptr<TypeNode> SemanticAnalysis::visit(VariableRefNode *node){
    auto symbol = currentScope->lookup(node->name);
    if(!symbol){
        errorHandler.errorReg("Variable '" + node->name + "' not defined.", 0);
        return nullptr;
    }
    if(symbol->kind != SymbolKind::VAR){
        errorHandler.errorReg("'" + node->name + "' is not a variable.", 0);
        return nullptr;
    }
    node->symbol = symbol;
    node->type = symbol->type;
    return symbol->type;
}
std::shared_ptr<TypeNode> SemanticAnalysis::visit(ArrayRefNode *node){
    auto symbol = currentScope->lookup(node->name);
    if(!symbol){
        errorHandler.errorReg("Array '" + node->name + "' not defined.", 0);
        return nullptr;
    }
    if(symbol->kind != SymbolKind::ARRAY){
        errorHandler.errorReg("'" + node->name + "' is not a array.", 0);
        return nullptr;
    }
    node->symbol = symbol;

    // 配列の要素型を取得
    std::string arrayTypeName = symbol->type->getTypeName(); // 例: "int[5]"
    size_t bracketPos = arrayTypeName.find('[');
    if (bracketPos == std::string::npos) {
        errorHandler.errorReg("Internal error: Array symbol type name does not contain brackets.", 0);
        return nullptr;
    }
    std::string elementTypeName = arrayTypeName.substr(0, bracketPos); // 例: "int"

    std::shared_ptr<TypeNode> elementType = getType(elementTypeName); // 基本型マップから取得
    if (!elementType) {
        errorHandler.errorReg("Internal error: Could not determine element type for array '" + node->name + "'.", 0);
        return nullptr;
    }
    node->type = elementType; // ArrayRefNodeの型は要素型

    // インデックス式の型をチェック
    std::shared_ptr<TypeNode> indexType = visit(node->idx.get());
    if (!indexType || indexType->getTypeName() != "int") {
        errorHandler.errorReg("Array index must be an integer for array '" + node->name + "'.", 0);
        return nullptr;
    }

    return elementType;
}
std::shared_ptr<TypeNode> SemanticAnalysis::visit(CastNode *node){
    std::shared_ptr<TypeNode> exprType = visit(node->expression.get());
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
    return node->type;
}

// 式以外
void SemanticAnalysis::visit(FunctionDefNode *node){
    // FuncSymbol作成
    auto returnType = node->returnType;
    auto funcSymbol = std::make_shared<FuncSymbol>(node->name, returnType, currentScope);
    // 関数定義
    if(!currentScope->define(funcSymbol)){
        errorHandler.errorReg("Function '" + node->name + "' already defined.", 0);
        return;
    }
    // ASTノードにシンボル紐づけ
    node->symbol = funcSymbol;

    // 以前の戻り値の型を保存
    auto prevReturnType = currentFunctionReturnType;
    currentFunctionReturnType = node->returnType.get();

    // 関数用の新しいスコープに入る
    auto prevFunction = currentFunction;
    currentFunction = funcSymbol;
    enterScope();
    funcSymbol->funcScope = currentScope;
    // 引数を定義
    for(size_t i = 0;i < node->args.size();i++){
        auto argType = node->argTypes[i];
        auto argSymbol = std::make_shared<VarSymbol>(node->args[i], argType, currentScope);
        if(!currentScope->define(argSymbol)){
            errorHandler.errorReg("Argument '" + node->args[i] + "' redefined.", 0);
        }
        funcSymbol->addParameter(argSymbol);
    }
    // 関数本体を解析
    visit(node->body.get());
    // スコープとコンテキストを抜ける
    leaveScope();
    currentFunction = prevFunction;
    // 以前の戻り値の型に戻す
    currentFunctionReturnType = prevReturnType;
}
void SemanticAnalysis::visit(ProgramNode *node){
    for(auto stmt : node->statements){
        visit(stmt.get());
    }
}
void SemanticAnalysis::visit(BlockNode *node){
    enterScope();
    for(auto stmt : node->statements){
        visit(stmt.get());
    }
    leaveScope();
}
void SemanticAnalysis::visit(IfNode *node){
    std::shared_ptr<TypeNode> condType = visit(node->condition.get());
    if(condType->getTypeName() != "bool"){
        errorHandler.errorReg(ErrorCode::FOR_NOT_BOOL, {condType->getTypeName()});
        return;
    }
    visit(node->if_block.get());
    if(node->else_block){
        visit(node->else_block.get());
    }
}
void SemanticAnalysis::visit(ForNode *node){
    std::shared_ptr<TypeNode> condType = visit(node->condition.get());
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
        TypeNode* returnExprType = visit(node->returnValue.get()).get();
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
    if(auto cnode = dynamic_cast<FunctionDefNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<IfNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ForNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ReturnNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<VarDeclNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<AssignmentNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ExprStatementNode*>(node)) return visit(cnode);
    if(auto cnode = dynamic_cast<ArrayDeclNode*>(node)) return visit(cnode);
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
#include "SemanticAnalysis.h"
#include <iostream>

SemanticAnalysis::SemanticAnalysis() {
    // Global scope
    enterScope();
}

void SemanticAnalysis::analyze(ProgramNode* root) {
    visit(root);
}

void SemanticAnalysis::enterScope() {
    scopes.emplace_back();
}

void SemanticAnalysis::leaveScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void SemanticAnalysis::addSymbol(const std::string& name, std::shared_ptr<TypeNode> type) {
    if (scopes.empty()) {
        errorHandler.errorReg("Cannot add symbol to empty scope stack.", 0);
        return;
    }
    // Check for duplicate in current scope
    if (scopes.back().count(name)) {
        errorHandler.errorReg("Redeclaration of variable '" + name + "' in the same scope.", 0);
        return;
    }
    scopes.back()[name] = type;
}

std::shared_ptr<TypeNode> SemanticAnalysis::findSymbol(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->count(name)) {
            return it->at(name);
        }
    }
    return nullptr; // Not found
}

void SemanticAnalysis::visit(ProgramNode* node) {
    for (const auto& stmt : node->statements) {
        visit(stmt.get());
    }
}

void SemanticAnalysis::visit(StatementNode* node) {
    // This is a generic visitor for statements. Specific statement types will override this.
    // For now, we'll dynamic_cast to known statement types.
    if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        visit(varDecl);
    } else if (auto funcDef = dynamic_cast<FunctionDefNode*>(node)) {
        visit(funcDef);
    } else if (auto exprStmt = dynamic_cast<ExprStatementNode*>(node)) {
        visit(exprStmt);
    } else if (auto assignment = dynamic_cast<AssignmentNode*>(node)) {
        visit(assignment);
    } else if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        visit(ifNode);
    } else if (auto forNode = dynamic_cast<ForNode*>(node)) {
        visit(forNode);
    } else {
        // Handle unknown statement type or add more specific visitors
    }
}

void SemanticAnalysis::visit(BlockNode* node) {
    enterScope();
    for (const auto& stmt : node->statements) {
        visit(stmt.get());
    }
    leaveScope();
}

void SemanticAnalysis::visit(FunctionDefNode* node) {
    // TODO: Handle function parameters and return types
    enterScope();
    // Add parameters to the function's scope
    for (const auto& paramName : node->parameters) {
        // For now, assume all parameters are int32. This needs to be improved.
        addSymbol(paramName, std::make_shared<BasicTypeNode>("int32"));
    }
    visit(node->body.get());
    leaveScope();
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(VarDeclNode* node) {
    std::shared_ptr<TypeNode> resolvedType = nullptr;

    // 1. Determine type
    if (node->type) {
        // Type annotation exists
        resolvedType = node->type;
    } else if (node->initializer) {
        // Infer type from initializer
        resolvedType = visit(node->initializer.get());
        if (!resolvedType) {
            errorHandler.errorReg("Could not infer type for variable '" + node->varName + "' from initializer.", 0);
            return nullptr;
        }
    } else {
        // Default to int64 if no type annotation and no initializer
        resolvedType = std::make_shared<BasicTypeNode>("int64");
        errorHandler.errorReg("Variable '" + node->varName + "' declared without type or initializer; defaulting to int64.", 1); // Warning
    }

    // 2. Check for duplicate in current scope
    if (scopes.empty()) {
        errorHandler.errorReg("Internal error: Scope stack is empty during variable declaration.", 0);
        return nullptr;
    }
    if (scopes.back().count(node->varName)) {
        errorHandler.errorReg("Redeclaration of variable '" + node->varName + "' in the same scope.", 0);
        return nullptr;
    }

    // 3. Register in symbol table
    addSymbol(node->varName, resolvedType);

    // 4. Write to AST
    node->type = resolvedType;
    return resolvedType;
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(VariableRefNode* node) {
    std::shared_ptr<TypeNode> foundType = findSymbol(node->name);
    if (!foundType) {
        errorHandler.errorReg("Undefined variable '" + node->name + "'.", 0);
        // Return a special error type or nullptr
        return nullptr; 
    }
    // Write to AST
    node->type = foundType;
    return foundType;
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(BinaryOpNode* node) {
    std::shared_ptr<TypeNode> leftType = visit(node->left.get());
    std::shared_ptr<TypeNode> rightType = visit(node->right.get());

    if (!leftType || !rightType) {
        // Error already reported by sub-expressions
        return nullptr;
    }

    // Type checking rules
    // For simplicity, let's assume arithmetic operations for now
    if (leftType->isInteger() && rightType->isInteger()) {
        // Result is integer
        node->type = std::make_shared<BasicTypeNode>("int32"); // Assuming int32 for now
        return node->type;
    } else if (leftType->isNumeric() && rightType->isNumeric()) {
        // If either is float, result is float
        node->type = std::make_shared<BasicTypeNode>("float32"); // Assuming float32 for now
        return node->type;
    } else {
        errorHandler.errorReg("Type mismatch in binary operation for operator '" + node->op + "'.", 0);
        return nullptr;
    }
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(NumberLiteralNode* node) {
    // For simplicity, assume all number literals are int32 for now
    // In a real compiler, you'd differentiate based on value or suffix (e.g., 1.0f)
    node->type = std::make_shared<BasicTypeNode>("int32");
    return node->type;
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(ExprNode* node) {
    // Generic expression visitor, needs to be dynamic_cast to specific types
    if (auto numLit = dynamic_cast<NumberLiteralNode*>(node)) {
        return visit(numLit);
    } else if (auto binOp = dynamic_cast<BinaryOpNode*>(node)) {
        return visit(binOp);
    } else if (auto varRef = dynamic_cast<VariableRefNode*>(node)) {
        return visit(varRef);
    } else if (auto funcCall = dynamic_cast<FunctionCallNode*>(node)) {
        return visit(funcCall);
    }
    errorHandler.errorReg("Unknown expression type encountered during semantic analysis.", 0);
    return nullptr;
}

void SemanticAnalysis::visit(ExprStatementNode* node) {
    visit(node->expression.get());
}

void SemanticAnalysis::visit(AssignmentNode* node) {
    // TODO: Implement type checking for assignment
    visit(node->value.get());
}

void SemanticAnalysis::visit(IfNode* node) {
    // TODO: Implement type checking for if condition and blocks
    visit(node->condition.get());
    visit(node->if_block.get());
    if (node->else_block) {
        visit(node->else_block.get());
    }
}

void SemanticAnalysis::visit(ForNode* node) {
    // TODO: Implement type checking for for loop components
    visit(node->condition.get());
    visit(node->block.get());
}

std::shared_ptr<TypeNode> SemanticAnalysis::visit(FunctionCallNode* node) {
    // TODO: Implement type checking for function calls
    for (const auto& arg : node->args) {
        visit(arg.get());
    }
    // For now, assume print returns void and input returns int32
    if (node->calleeName == "print") {
        return nullptr; // Representing void
    } else if (node->calleeName == "input") {
        return std::make_shared<BasicTypeNode>("int32");
    }
    errorHandler.errorReg("Unknown function call '" + node->calleeName + "' during semantic analysis.", 0);
    return nullptr;
}

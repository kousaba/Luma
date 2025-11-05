#ifndef LUMA_SEMANTIC_ANALYSIS_H
#define LUMA_SEMANTIC_ANALYSIS_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "../ast/AstNode.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
#include "../ast/Definition.h"
#include "../common/ErrorHandler.h"
#include "../types/Type.h" // Assuming Type.h defines TypeNode and its subclasses

// Forward declarations for AST nodes
class ProgramNode;
class BlockNode;
class FunctionDefNode;
class VarDeclNode;
class VariableRefNode;
class BinaryOpNode;
class NumberLiteralNode;
class ExprStatementNode;
class AssignmentNode;
class IfNode;
class ForNode;
class FunctionCallNode;

class SemanticAnalysis {
public:
    SemanticAnalysis();
    void analyze(ProgramNode* root);

    // Scope management
    void enterScope();
    void leaveScope();
    void addSymbol(const std::string& name, std::shared_ptr<TypeNode> type);
    std::shared_ptr<TypeNode> findSymbol(const std::string& name);

    // Visit methods for AST nodes
    void visit(ProgramNode* node);
    void visit(StatementNode* node); // Generic statement visitor
    void visit(BlockNode* node);
    void visit(FunctionDefNode* node);
    std::shared_ptr<TypeNode> visit(ExprNode* node); // Generic expression visitor
    std::shared_ptr<TypeNode> visit(VarDeclNode* node);
    std::shared_ptr<TypeNode> visit(VariableRefNode* node);
    std::shared_ptr<TypeNode> visit(BinaryOpNode* node);
    std::shared_ptr<TypeNode> visit(NumberLiteralNode* node);
    void visit(ExprStatementNode* node);
    void visit(AssignmentNode* node);
    void visit(IfNode* node);
    void visit(ForNode* node);
    std::shared_ptr<TypeNode> visit(FunctionCallNode* node);

private:
    std::vector<std::map<std::string, std::shared_ptr<TypeNode>>> scopes;
    ErrorHandler errorHandler;
};

#endif // LUMA_SEMANTIC_ANALYSIS_H

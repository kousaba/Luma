#pragma once
#include "LumaParser.h"
#include "../ast/AstNode.h"
#include "LumaParserVisitor.h"

class AstBuilder : public Luma::LumaParserVisitor {
public:
    // program
    antlrcpp::Any visitProgram(Luma::LumaParser::ProgramContext *ctx) override;
    // statement
    antlrcpp::Any visitStatement(Luma::LumaParser::StatementContext *ctx) override;
    // block
    antlrcpp::Any visitBlock(Luma::LumaParser::BlockContext *ctx) override;
    // func
    antlrcpp::Any visitFunctionDefinition(Luma::LumaParser::FunctionDefinitionContext *ctx) override;
    antlrcpp::Any visitParameterList(Luma::LumaParser::ParameterListContext *ctx) override;
    // return
    antlrcpp::Any visitReturnStatement(Luma::LumaParser::ReturnStatementContext *ctx) override;
    // varDecl
    antlrcpp::Any visitVarDecl(Luma::LumaParser::VarDeclContext *ctx) override;
    antlrcpp::Any visitTypeAnnotation(Luma::LumaParser::TypeAnnotationContext *ctx) override;
    antlrcpp::Any visitTypeName(Luma::LumaParser::TypeNameContext *ctx) override;
    // assignment
    antlrcpp::Any visitAssignmentStatement(Luma::LumaParser::AssignmentStatementContext *ctx) override;
    // if
    antlrcpp::Any visitIfStatement(Luma::LumaParser::IfStatementContext *ctx) override;
    // for
    antlrcpp::Any visitConditionForStatement(Luma::LumaParser::ConditionForStatementContext *ctx) override;
    
    // expr
    antlrcpp::Any visitPrimaryExpr(Luma::LumaParser::PrimaryExprContext *ctx) override;
    antlrcpp::Any visitCastExpr(Luma::LumaParser::CastExprContext *ctx) override;
    antlrcpp::Any visitFunctionCallExpr(Luma::LumaParser::FunctionCallExprContext *ctx) override;
    antlrcpp::Any visitArgList(Luma::LumaParser::ArgListContext *ctx) override;
    antlrcpp::Any visitAdditiveExpr(Luma::LumaParser::AdditiveExprContext *ctx) override;
    antlrcpp::Any visitMultiplicativeExpr(Luma::LumaParser::MultiplicativeExprContext *ctx) override;
    antlrcpp::Any visitComparisonExpr(Luma::LumaParser::ComparisonExprContext *ctx) override;
    antlrcpp::Any visitExpr(Luma::LumaParser::ExprContext *ctx) override;
};
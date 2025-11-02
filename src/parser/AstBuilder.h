#pragma once
#include "LumaParser.h"
#include "../ast/AstNode.h" // 作成したASTノードのヘッダ
#include "LumaParserVisitor.h"

class AstBuilder : public Luma::LumaParserVisitor {
public:
    // program
    antlrcpp::Any visitProgram(Luma::LumaParser::ProgramContext *ctx) override;
    // statement
    antlrcpp::Any visitStatement(Luma::LumaParser::StatementContext *ctx) override;
    // varDecl
    antlrcpp::Any visitVarDecl(Luma::LumaParser::VarDeclContext *ctx) override;
    // assignment
    antlrcpp::Any visitAssignmentStatement(Luma::LumaParser::AssignmentStatementContext *ctx) override;
    
    // expr
    antlrcpp::Any visitPrimaryExpr(Luma::LumaParser::PrimaryExprContext *ctx) override;
    antlrcpp::Any visitAdditiveExpr(Luma::LumaParser::AdditiveExprContext *ctx) override;
    antlrcpp::Any visitMultiplicativeExpr(Luma::LumaParser::MultiplicativeExprContext *ctx) override;
    antlrcpp::Any visitComparisonExpr(Luma::LumaParser::ComparisonExprContext *ctx) override;
    antlrcpp::Any visitExpr(Luma::LumaParser::ExprContext *ctx) override;
};
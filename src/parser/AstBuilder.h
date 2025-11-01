#pragma once
#include "ast/AstNode.h" // 作成したASTノードのヘッダ
#include "LumaParserVisitor.h"

class AstBuilder : public Luma::LumaParserVisitor {
public:
    // programルールに対応するvisitメソッド
    antlrcpp::Any visitProgram(Luma::LumaParser::ProgramContext *ctx) override;

    // statementルールに対応するvisitメソッド
    antlrcpp::Any visitStatement(Luma::LumaParser::StatementContext *ctx) override;

    // variableDeclarationルールに対応するvisitメソッド
    antlrcpp::Any visitVarDecl(Luma::LumaParser::VarDeclContext *ctx) override;
};
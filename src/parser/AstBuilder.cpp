#pragma once
#include "AstBuilder.h"
#include "LumaLexer.h"
#include "LumaParser.h"
#include "LumaParserVisitor.h"
#include "Token.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
#include "../ast/Definition.h"
#include "common/Global.h"
#include <cstddef>
#include <iostream> // デバッグ用

antlrcpp::Any AstBuilder::visitProgram(Luma::LumaParser::ProgramContext *ctx) {
    auto progNode = std::make_shared<ProgramNode>();

    for (auto stmtCtx : ctx->statement()) {
        antlrcpp::Any result = visit(stmtCtx);
        if (result.has_value()) {
            progNode->statements.push_back(std::any_cast<std::shared_ptr<StatementNode>>(result));
        }
    }

    antlrcpp::Any finalResult = progNode;
    return finalResult;
}

antlrcpp::Any AstBuilder::visitStatement(Luma::LumaParser::StatementContext *ctx) {
    if(ctx->varDecl()) return visit(ctx->varDecl());
    if(ctx->assignmentStatement()) return visit(ctx->assignmentStatement());
    if(ctx->block()) return visit(ctx->block());
    if(ctx->ifStatement()) return visit(ctx->ifStatement());
    if(ctx->forStatement()) return visit(ctx->forStatement());
    if(ctx->functionDefinition()) return visit(ctx->functionDefinition());
    if(ctx->expr()){
        antlrcpp::Any exprAny = visit(ctx->expr());
        auto exprNode = std::any_cast<std::shared_ptr<ExprNode>>(exprAny);
        auto stmtNode = std::make_shared<ExprStatementNode>(exprNode);
        return std::shared_ptr<StatementNode>(stmtNode);
    }
    return {};
}

antlrcpp::Any AstBuilder::visitBlock(Luma::LumaParser::BlockContext *ctx){
    auto blockNode = std::make_shared<BlockNode>();
    for(auto stmtCtx : ctx->statement()){
        antlrcpp::Any result = visit(stmtCtx);
        if(result.has_value()){
            blockNode->statements.push_back(std::any_cast<std::shared_ptr<StatementNode>>(result));
        }
    }
    antlrcpp::Any finalResult = blockNode;
    return finalResult;
}

antlrcpp::Any AstBuilder::visitFunctionDefinition(Luma::LumaParser::FunctionDefinitionContext *ctx){
    std::string funcName = ctx->IDENTIFIER()->getText();
    std::vector<std::string> args;
    if(ctx->parameterList()) for(const auto& argCtx : ctx->parameterList()->IDENTIFIER()) args.push_back(argCtx->getText());
    std::shared_ptr<BlockNode> body = std::any_cast<std::shared_ptr<BlockNode>>(visitBlock(ctx->block()));
    auto node = std::make_shared<FunctionDefNode>(funcName, args, body);
    antlrcpp::Any result = std::shared_ptr<StatementNode>(node);
    return result;
}
antlrcpp::Any AstBuilder::visitParameterList(Luma::LumaParser::ParameterListContext *ctx){
    return nullptr;
}
antlrcpp::Any AstBuilder::visitReturnStatement(Luma::LumaParser::ReturnStatementContext *ctx){
    // TOOD: return処理
    return nullptr;
}

antlrcpp::Any AstBuilder::visitVarDecl(Luma::LumaParser::VarDeclContext *ctx) {
    std::string varName = ctx->IDENTIFIER()->getText();
    std::shared_ptr<ExprNode> init = nullptr;
    if(ctx->expr()){
        antlrcpp::Any initAny = visit(ctx->expr());
        init = std::any_cast<std::shared_ptr<ExprNode>>(initAny);
    }
    auto node = std::make_shared<VarDeclNode>(varName, init);
    antlrcpp::Any result = std::shared_ptr<StatementNode>(node);
    return result;
}

antlrcpp::Any AstBuilder::visitAssignmentStatement(Luma::LumaParser::AssignmentStatementContext *ctx){
    std::string varName = ctx->IDENTIFIER()->getText();
    antlrcpp::Any valAny = visit(ctx->expr());
    std::shared_ptr<ExprNode> val = std::any_cast<std::shared_ptr<ExprNode>>(valAny);
    auto node = std::make_shared<AssignmentNode>(varName, val);
    antlrcpp::Any result = std::shared_ptr<StatementNode>(node);
    return result;
}

antlrcpp::Any AstBuilder::visitIfStatement(Luma::LumaParser::IfStatementContext *ctx){
    antlrcpp::Any conditionAny = visit(ctx->expr());
    antlrcpp::Any ifblockAny = visit(ctx->block(0));
    antlrcpp::Any elseblockAny = visit(ctx->block(1));
    std::shared_ptr<BlockNode> elseblock = nullptr;
    std::shared_ptr<ExprNode> condition = std::any_cast<std::shared_ptr<ExprNode>>(conditionAny);
    std::shared_ptr<BlockNode> ifblock = std::any_cast<std::shared_ptr<BlockNode>>(ifblockAny);
    if(ctx->block(1)) elseblock = std::any_cast<std::shared_ptr<BlockNode>>(elseblockAny);
    auto node = std::make_shared<IfNode>(condition, ifblock, elseblock);
    antlrcpp::Any result = std::shared_ptr<StatementNode>(node);
    return result;
}

antlrcpp::Any AstBuilder::visitConditionForStatement(Luma::LumaParser::ConditionForStatementContext *ctx){
    antlrcpp::Any conditionAny = visit(ctx->expr());
    antlrcpp::Any blockAny = visit(ctx->block());
    std::shared_ptr<ExprNode> condition = std::any_cast<std::shared_ptr<ExprNode>>(conditionAny);
    std::shared_ptr<BlockNode> block = std::any_cast<std::shared_ptr<BlockNode>>(blockAny);
    auto node = std::make_shared<ForNode>(condition, block);
    antlrcpp::Any result = std::shared_ptr<StatementNode>(node);
    return result;
}

antlrcpp::Any AstBuilder::visitPrimaryExpr(Luma::LumaParser::PrimaryExprContext *ctx){
    if(ctx->INTEGER()){
        int val = std::stoi(ctx->INTEGER()->getText());
        auto node = std::make_shared<NumberLiteralNode>(val);
        antlrcpp::Any result = std::shared_ptr<ExprNode>(node);
        return result;
    }else if(ctx->expr()){
        return visit(ctx->expr());
    }else if(ctx->IDENTIFIER()){
        std::string varName = ctx->IDENTIFIER()->getText();
        auto node = std::make_shared<VariableRefNode>(varName);
        antlrcpp::Any result = std::shared_ptr<ExprNode>(node);
        return result;
    }else if(ctx->functionCallExpr()){
        return visit(ctx->functionCallExpr());
    }else{
        errorHandler.errorReg("Unknown primary expr.", 0);
        return nullptr;
    }
}

antlrcpp::Any AstBuilder::visitFunctionCallExpr(Luma::LumaParser::FunctionCallExprContext *ctx){
    std::string funcName = ctx->IDENTIFIER()->getText();
    std::vector<std::shared_ptr<ExprNode>> args;
    if(ctx->argList()){
        for(const auto& exprCtx : ctx->argList()->expr()){
            antlrcpp::Any argAny = visit(exprCtx);
            std::shared_ptr<ExprNode> arg = std::any_cast<std::shared_ptr<ExprNode>>(argAny);
            args.push_back(arg);
        }
    }
    auto node = std::make_shared<FunctionCallNode>(funcName, args);
    return std::shared_ptr<ExprNode>(node);
}

antlrcpp::Any AstBuilder::visitArgList(Luma::LumaParser::ArgListContext *ctx){
    // 現在は何も実行するものがない(ctx->argList()->arg)などで呼び出しているので
    // 実行するものがない
    // とりあえずnullptrを返す
    return nullptr;
}

antlrcpp::Any AstBuilder::visitAdditiveExpr(Luma::LumaParser::AdditiveExprContext *ctx){
    // 1. 最初の左辺を取得
    antlrcpp::Any lhs_any = visit(ctx->multiplicativeExpr(0));

    // 2. 演算子の数だけループ (ctx->opはTokenのリストになる)
    for (size_t i = 0; i < ctx->op.size(); ++i) {
        // 3. 演算子のテキストを取得
        std::string op_text = ctx->op[i]->getText();

        // 4. 対応する右辺を取得
        antlrcpp::Any rhs_any = visit(ctx->multiplicativeExpr(i + 1));

        // キャスト
        auto lhs_expr = std::any_cast<std::shared_ptr<ExprNode>>(lhs_any);
        auto rhs_expr = std::any_cast<std::shared_ptr<ExprNode>>(rhs_any);

        // 新しい左辺を構築
        auto new_lhs_expr = std::make_shared<BinaryOpNode>(op_text, lhs_expr, rhs_expr);
        
        // 次のループのために更新
        lhs_any = std::shared_ptr<ExprNode>(new_lhs_expr);
    }

    // 最終的な結果を返す
    return lhs_any;
}

antlrcpp::Any AstBuilder::visitMultiplicativeExpr(Luma::LumaParser::MultiplicativeExprContext *ctx) {
    // 1. 最初の左辺を取得
    antlrcpp::Any lhs_any = visit(ctx->primaryExpr(0));

    // 2. 演算子の数だけループ (ctx->opはTokenのリストになる)
    for (size_t i = 0; i < ctx->op.size(); ++i) {
        // 3. 演算子のテキストを取得
        std::string op_text = ctx->op[i]->getText();

        // 4. 対応する右辺を取得
        antlrcpp::Any rhs_any = visit(ctx->primaryExpr(i + 1));

        // キャスト
        auto lhs_expr = std::any_cast<std::shared_ptr<ExprNode>>(lhs_any);
        auto rhs_expr = std::any_cast<std::shared_ptr<ExprNode>>(rhs_any);

        // 新しい左辺を構築
        auto new_lhs_expr = std::make_shared<BinaryOpNode>(op_text, lhs_expr, rhs_expr);
        
        // 次のループのために更新
        lhs_any = std::shared_ptr<ExprNode>(new_lhs_expr);
    }

    // 最終的な結果を返す
    return lhs_any;
}

antlrcpp::Any AstBuilder::visitComparisonExpr(Luma::LumaParser::ComparisonExprContext *ctx){
    antlrcpp::Any lhs_any = visit(ctx->additiveExpr(0));
    if(ctx->op){
        std::string op_text = ctx->op->getText();
        antlrcpp::Any rhs_any = visit(ctx->additiveExpr(1));

        auto lhs_expr = std::any_cast<std::shared_ptr<ExprNode>>(lhs_any);
        auto rhs_expr = std::any_cast<std::shared_ptr<ExprNode>>(rhs_any);

        auto new_expr = std::make_shared<BinaryOpNode>(op_text, lhs_expr, rhs_expr);
        antlrcpp::Any result = std::shared_ptr<ExprNode>(new_expr);
        return result;
    }
    return lhs_any;
}

antlrcpp::Any AstBuilder::visitExpr(Luma::LumaParser::ExprContext *ctx){
    return visit(ctx->comparisonExpr());
}
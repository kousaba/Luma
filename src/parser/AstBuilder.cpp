#pragma once
#include "AstBuilder.h"
#include "LumaLexer.h"
#include "LumaParser.h"
#include "LumaParserVisitor.h"
#include "Token.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
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
    antlrcpp::Any result = visitChildren(ctx); // 子を訪問
    return result;
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
    return nullptr;
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
    }else{
        std::cerr << "Error: Unknown primary expr.\n";
        return nullptr;
    }
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
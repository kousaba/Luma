#include "parser/AstBuilder.h"
#include "ast/Statement.h"
#include <iostream> // デバッグ用

antlrcpp::Any AstBuilder::visitProgram(Luma::LumaParser::ProgramContext *ctx) {
    std::cout << "[ENTER] visitProgram\n";
    
    auto progNode = std::make_shared<ProgramNode>();

    for (auto stmtCtx : ctx->statement()) {
        antlrcpp::Any result = visit(stmtCtx);
        if (result.has_value()) {
            progNode->statements.push_back(std::any_cast<std::shared_ptr<StatementNode>>(result));
        }
    }
    
    // ★★★ 戻り値の型をコンソールに出力 ★★★
    antlrcpp::Any finalResult = progNode;
    std::cout << "[EXIT]  visitProgram, returning type: " << finalResult.type().name() << "\n";
    return finalResult;
}

antlrcpp::Any AstBuilder::visitStatement(Luma::LumaParser::StatementContext *ctx) {
    std::cout << "[ENTER] visitStatement\n";
    
    antlrcpp::Any result = visitChildren(ctx); // 子を訪問

    // ★★★ 戻り値の型をコンソールに出力 ★★★
    std::cout << "[EXIT]  visitStatement, returning type: " << (result.has_value() ? result.type().name() : "empty") << "\n";
    return result;
}

antlrcpp::Any AstBuilder::visitVarDecl(Luma::LumaParser::VarDeclContext *ctx) {
    std::cout << "[ENTER] visitVarDecl\n";

    std::string varName = ctx->IDENTIFIER()->getText();
    auto node = std::make_shared<VarDeclNode>(varName);
    
    // ★★★ 戻り値の型をコンソールに出力 ★★★
    antlrcpp::Any result = std::shared_ptr<StatementNode>(node);
    std::cout << "[EXIT]  visitVarDecl, returning type: " << result.type().name() << "\n";
    return result;
}

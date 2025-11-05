// #include "ast/AstNode.h"
// #include "ast/Statement.h"
// #include "types/Type.h"
// #include <map>
// #include <memory>
// #include <string>


// // シンボルテーブル
// using SymbolTable = std::map<std::string, std::shared_ptr<TypeNode>>;

// class SemanticAnalyzer{
// private:
//     // スコープの管理をするためのstack
//     std::vector<SymbolTable> scopes;
    
// public:
//     // スコープを管理するヘルパーメソッド
//     void enterScope();
//     void leaveScope();
//     // 変数を現在のスコープに登録
//     void addSymbol(const std::string& name, std::shared_ptr<TypeNode> type);
//     // 現在のスコープから内側->外側の順に変数を探す
//     std::shared_ptr<TypeNode> findSymbol(const std::string& name);
//     // ASTノードごとのvisitメソッド
//     void visit(ProgramNode *node);
//     void visit(BlockNode *node);
//     void visit(VarDeclNode *node);
//     // 式のviistメソッドは、その式の型を返す
//     std::shared_ptr<TypeNode> visit(ExprNode *node);
//     std::shared_ptr<TypeNode> visit(NumberLiteralNode *node);
//     std::shared_ptr<TypeNode> visit(BinaryOpNode *node);

// };
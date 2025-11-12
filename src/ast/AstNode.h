#pragma once
#include <string>
#include <iostream> // std::cout, std::endl を使用するため

// インデントを出力するヘルパー関数
inline void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  "; // スペース2つでインデント
    }
}

class AstNode{
public:
    virtual ~AstNode() = default;
    // ASTノードの情報を出力する仮想メソッド
    virtual void dump(int indent = 0) const = 0;
};
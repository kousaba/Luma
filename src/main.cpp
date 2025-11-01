#include <iostream>
#include <fstream>
#include <memory>

// ANTLR
#include "CommonTokenStream.h"
#include "LumaParser.h"
#include "antlr4-runtime.h"
#include "LumaLexer.h"
#include "LumaParser.h"

// LLVM
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

int main(int argc, char* argv[]){
    // コマンドライン引数の解析
    if(argc != 2){
        std::cerr << "Please write the executable file name and source file name.\n";
        return 1;
    }
    // ソースファイルの読み込み
    std::ifstream file(argv[1]);
    if(!file){
        std::cerr << "The file could not be opened.\n";
        return 1;
    }
    // パース処理
    antlr4::ANTLRInputStream inputStream(file);
    // Lexer生成
    Luma::LumaLexer lexer(&inputStream);
    // トークンストリームを生成
    antlr4::CommonTokenStream tokens(&lexer);
    // Parserを生成
    Luma::LumaParser parser(&tokens);
}
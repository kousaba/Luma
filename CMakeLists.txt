# --- 1. プロジェクトの基本設定 ---
cmake_minimum_required(VERSION 3.20)
project(Luma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 2. ANTLRのコード生成設定 ---
# ANTLRの実行に必要なJavaと、ANTLRのjarファイルを見つける
find_package(Java REQUIRED)
set(ANTLR4_JAR_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/external/antlr-4.13.2-complete.jar")

# ANTLRの文法ファイル
set(ANTLR_GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaParser.g4)
set(ANTLR_LEXER_GRAMMAR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.g4)

# 生成されるファイルのリストを定義
set(ANTLR_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/generated)
set(ANTLR_GENERATED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.cpp
    ${ANTLR_GENERATED_DIR}/LumaParser.cpp  # Visitorパターンを使うことを想定
)
set(ANTLR_GENERATED_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.h
    ${ANTLR_GENERATED_DIR}/LumaParser.h
)

file(MAKE_DIRECTORY ${ANTLR_GENERATED_DIR})

# ANTLR Lexerコードを生成するカスタムコマンドを定義
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.h
           ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.interp
           ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.tokens
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR4_JAR_EXECUTABLE}
            -Dlanguage=Cpp
            -o ${CMAKE_CURRENT_SOURCE_DIR}/grammar # Output directory for LumaLexer.g4
            -visitor
            -package Luma
            ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.g4
    COMMENT "Generating ANTLR Lexer for Luma"
    VERBATIM
)

# ANTLR Parser/Visitorコードを生成するカスタムコマンドを定義
add_custom_command(
    OUTPUT ${ANTLR_GENERATED_DIR}/LumaParser.cpp
           ${ANTLR_GENERATED_DIR}/LumaParser.h
           ${ANTLR_GENERATED_DIR}/LumaParser.interp
           ${ANTLR_GENERATED_DIR}/LumaParser.tokens
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR4_JAR_EXECUTABLE}
            -Dlanguage=Cpp
            -o ${ANTLR_GENERATED_DIR} # Output directory for LumaParser.g4
            -visitor
            -package Luma
            ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaParser.g4
    COMMENT "Generating ANTLR Parser and Visitor for Luma"
    VERBATIM
)


# LLVMの設定 (llvm-config を使用)
find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config-18 llvm-config)

if(NOT LLVM_CONFIG_EXECUTABLE)
    message(FATAL_ERROR "Could not find llvm-config-18 or llvm-config. Please ensure LLVM 18 is installed and in your PATH.")
endif()

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
                OUTPUT_VARIABLE LLVM_INCLUDE_DIRS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core irreader support
                OUTPUT_VARIABLE LLVM_LIBS
                OUTPUT_STRIP_TRAILING_WHITESPACE)




# 実行ファイルの定義
add_executable(Luma
    src/main.cpp
    src/parser/AstBuilder.cpp
    # src/CodeGen.cpp
    ${ANTLR_GENERATED_SOURCES} # 生成されたソースもコンパイル対象に
)



# Lumaターゲットにのみ設定を適用
target_include_directories(Luma PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${ANTLR_GENERATED_DIR}  # 生成されたヘッダのパス
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar # LumaLexer.h のパス
    ${CMAKE_CURRENT_SOURCE_DIR}/src # 自作ヘッダのパス
    ${CMAKE_CURRENT_SOURCE_DIR}/external/antlr4-cpp-runtime-4.13.2-source/runtime/src # ANTLR C++ runtime include
)

# target_compile_definitions(Luma PRIVATE ${LLVM_DEFINITIONS}) # LLVM_DEFINITIONS は find_package(LLVM) から来るためコメントアウト

# LLVMライブラリをリンク
target_link_libraries(Luma PRIVATE ${LLVM_LIBS} antlr4-runtime)

add_subdirectory(external/antlr4-cpp-runtime-4.13.2-source/runtime)
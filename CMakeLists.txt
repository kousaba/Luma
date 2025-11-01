# --- 1. プロジェクトの基本設定 ---
cmake_minimum_required(VERSION 3.20)
project(Luma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 2. 依存関係のセットアップ ---
add_subdirectory(external/antlr4-cpp-runtime-4.13.2-source/runtime)
find_package(Java REQUIRED)
set(ANTLR4_JAR_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/external/antlr-4.13.2-complete.jar")
find_package(LLVM 18.1 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# --- 3. ANTLRコード生成 ---
set(ANTLR_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/generated)

# 3a. Lexerの生成コマンド
# ====================================================================================
set(ANTLR_LEXER_GRAMMAR ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaLexer.g4)
set(ANTLR_LEXER_OUTPUTS
    ${ANTLR_GENERATED_DIR}/LumaLexer.cpp
    ${ANTLR_GENERATED_DIR}/LumaLexer.h
    ${ANTLR_GENERATED_DIR}/LumaLexer.interp
    ${ANTLR_GENERATED_DIR}/LumaLexer.tokens # Parserが依存する重要なファイル
)

add_custom_command(
    OUTPUT ${ANTLR_LEXER_OUTPUTS}
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR4_JAR_EXECUTABLE}
            -Dlanguage=Cpp -o ${ANTLR_GENERATED_DIR} -package Luma
            ${ANTLR_LEXER_GRAMMAR}
    DEPENDS ${ANTLR_LEXER_GRAMMAR}
    COMMENT "Generating ANTLR Lexer"
)

# 3b. Parserの生成コマンド
# ====================================================================================
set(ANTLR_PARSER_GRAMMAR ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LumaParser.g4)
set(ANTLR_PARSER_OUTPUTS
    ${ANTLR_GENERATED_DIR}/LumaParser.cpp
    ${ANTLR_GENERATED_DIR}/LumaParser.h
    ${ANTLR_GENERATED_DIR}/LumaParserVisitor.cpp
    ${ANTLR_GENERATED_DIR}/LumaParserVisitor.h
)

add_custom_command(
    OUTPUT ${ANTLR_PARSER_OUTPUTS}
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR4_JAR_EXECUTABLE}
            -Dlanguage=Cpp -o ${ANTLR_GENERATED_DIR} -visitor -package Luma
            ${ANTLR_PARSER_GRAMMAR}
    DEPENDS ${ANTLR_PARSER_GRAMMAR} ${ANTLR_GENERATED_DIR}/LumaLexer.tokens
    COMMENT "Generating ANTLR Parser"
)

# 3c. 生成される全ソースファイルをまとめる
set(ANTLR_GENERATED_SOURCES
    ${ANTLR_LEXER_OUTPUTS}
    ${ANTLR_PARSER_OUTPUTS}
)
add_custom_target(LumaAntlrGenerator DEPENDS ${ANTLR_GENERATED_SOURCES})

# --- 4. メインの実行可能ファイル "Luma" の定義 ---
add_executable(Luma
    src/main.cpp
    src/parser/AstBuilder.cpp
    src/codegen/CodeGen.cpp
    ${ANTLR_GENERATED_DIR}/LumaLexer.cpp   # 個別に指定
    ${ANTLR_GENERATED_DIR}/LumaParser.cpp
    ${ANTLR_GENERATED_DIR}/LumaParserVisitor.cpp
)

# --- 5. "Luma" ターゲットへの設定適用 ---
add_dependencies(Luma LumaAntlrGenerator)

target_include_directories(Luma PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${ANTLR_GENERATED_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/antlr4-cpp-runtime-4.13.2-source/runtime/src
)

target_compile_definitions(Luma PRIVATE ${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS core irreader support)
target_link_libraries(Luma PRIVATE
    antlr4-runtime
    ${LLVM_LIBS}
)
parser grammar LumaParser;
options{
    language = Cpp;
    tokenVocab = LumaLexer;
}

program: statement* EOF;

statement
    : varDecl
    | block
    | assignmentStatement
    | ifStatement
    | forStatement
    | expr SEMI // 普通の関数呼び出し
    | functionDefinition
    | returnStatement
    ;

block
    : LBRACE statement* RBRACE
    ;

varDecl
    : (VAR | LET) IDENTIFIER typeAnnotation? (EQ expr)? SEMI
    ;
typeAnnotation
    : COLON typeName
    ;
typeName
    : typeName LBRACKET INTEGER RBRACKET
    | TYPE_INT64 | TYPE_INT32 | TYPE_CHAR
    | TYPE_FLOAT64 | TYPE_FLOAT32
    ;
assignmentStatement
    : IDENTIFIER EQ expr SEMI
    ;

functionDefinition
    : FN IDENTIFIER '(' parameterList? ')' EQ typeName block
    ;
parameterList
    : (parameter ',')* parameter
    ;
parameter
    : IDENTIFIER typeAnnotation
    ;
returnStatement
    : RETURN expr? SEMI
    ;

ifStatement
    : IF expr block (ELSE block)?
    ;
forStatement
    : FOR expr block # ConditionForStatement
    ;

// 式ルール
// プライマリ式: 式の最小単位(数値リテラルなど)
primaryExpr
    : INTEGER
    | DECIMAL
    | IDENTIFIER LBRACKET expr RBRACKET // 配列アクセス
    | functionCallExpr
    | LPAREN expr RPAREN
    | IDENTIFIER
    ;

castExpr
    : primaryExpr (AS typeName)* // a as int as floatもok
    ;
// 関数呼び出し
functionCallExpr
    : IDENTIFIER LPAREN argList? RPAREN
    ;
// 引数リスト
argList
    : expr (COMMA expr)*
    ;
// 乗算・除算の式
multiplicativeExpr
    : castExpr ( op+=( MUL | DIV ) castExpr )*
    ;
// 加算・減算の式
additiveExpr
    : multiplicativeExpr ( op+=( ADD | SUB ) multiplicativeExpr )*
    ;
comparisonExpr
    : additiveExpr(op=(EQEQ | NE | LT | GT | LE | GE) additiveExpr)? // 比較は一回だけとする
    ;
// expressionルールのトップレベル
expr
    : comparisonExpr
    ;
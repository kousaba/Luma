parser grammar LumaParser;
options{
    language = Cpp;
    tokenVocab = LumaLexer;
}

program: statement* EOF;

statement
    : varDecl
    ;

varDecl
    : (VAR | LET) IDENTIFIER (EQ expr)? SEMI
    ;

// 式ルール
// プライマリ式: 式の最小単位(数値リテラルなど)
primaryExpr
    : INTEGER
    | LPAREN expr RPAREN
    ;
// 乗算・除算の式
multiplicativeExpr
    : primaryExpr ( op+=( MUL | DIV ) primaryExpr )*
    ;
// 加算・減算の式
additiveExpr
    : multiplicativeExpr ( op+=( ADD | SUB ) multiplicativeExpr )*
    ;
// expressionルールのトップレベル
expr
    : additiveExpr
    ;
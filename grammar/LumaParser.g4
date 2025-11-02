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
    ;

block
    : LBRACE statement* RBRACE
    ;

varDecl
    : (VAR | LET) IDENTIFIER (EQ expr)? SEMI
    ;
assignmentStatement
    : IDENTIFIER EQ expr SEMI
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
    | functionCallExpr
    | LPAREN expr RPAREN
    | IDENTIFIER
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
    : primaryExpr ( op+=( MUL | DIV ) primaryExpr )*
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


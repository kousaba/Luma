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
    | LPAREN expr RPAREN
    | IDENTIFIER
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
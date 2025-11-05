lexer grammar LumaLexer;
options{
    language=Cpp;
}

// キーワード
VAR: 'var';
LET: 'let';
IF: 'if';
ELSE: 'else';
FOR: 'for';
FN: 'fn';
RETURN: 'return';
AS: 'as';
// 型名
TYPE_INT64: 'int';
TYPE_INT32: 'i32';
TYPE_FLOAT64: 'float';
TYPE_FLOAT32: 'f32';
TYPE_CHAR: 'char';
// operator
EQ: '=';
ADD: '+';
SUB: '-';
MUL: '*';
DIV: '/';
GT: '>';
LT: '<';
EQEQ: '==';
NE: '!=';
GE: '>=';
LE: '<=';
// 記号
COLON: ':';
SEMI: ';';
COMMA: ',';
LPAREN: '(';
RPAREN: ')';
LBRACE: '{';
RBRACE: '}';
// リテラル
fragment DIGITS : '0'..'9'+;
DECIMAL: [+-]?DIGITS+'.'DIGITS+;
INTEGER: [+-]?DIGITS+;
IDENTIFIER: [a-zA-Z][a-zA-Z0-9]*;

// WS
WS: [ \t\n\r] -> skip;
lexer grammar LumaLexer;
options{
    language=Cpp;
}

// キーワード
VAR: 'var';
LET: 'let';
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
SEMI: ';';
LPAREN: '(';
RPAREN: ')';
// リテラル
INTEGER: [0-9]+;
IDENTIFIER: [a-zA-Z][a-zA-Z0-9]*;

// WS
WS: [ \t\n\r] -> skip;
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
LBRACE: '{';
RBRACE: '}';
// リテラル
INTEGER: [0-9]+;
IDENTIFIER: [a-zA-Z][a-zA-Z0-9]*;

// WS
WS: [ \t\n\r] -> skip;
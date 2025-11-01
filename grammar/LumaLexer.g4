lexer grammar LumaLexer;
options{
    language=Cpp;
}

// キーワード
VAR: 'var';
// 記号
SEMI: ';';
// リテラル
IDENTIFIER: [a-zA-Z0-9]+;

// WS
WS: [ \t] -> skip;
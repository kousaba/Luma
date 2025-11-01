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
    : VAR IDENTIFIER SEMI
    ;
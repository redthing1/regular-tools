/*
asm.h
provides lexer and parser for the assembler
*/

#pragma once

#include <stdio.h>

typedef enum {
    ALPHA = 1 << 0,   // abc
    NUMERIC = 1 << 1, // 123
    SPACE = 1 << 2,   // ' '
    ARGSEP = 1 << 3,  // ','
    MARK = 1 << 4,    // ':'
} TokenKind;

typedef struct {
    char *cont;
    TokenKind kind;
} Token;

typedef struct {
    Token *tokens;
    int tokenCount;
} LexResult;

LexResult lex(FILE *inf) {
    
}

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

typedef struct {
    char *buf;
    size_t size;
    int pos;
} LexerState;

inline char peek_char(LexerState *st) { return st->buf[st->pos]; }

inline char take_char(LexerState *st) {
    char c = peek_char(st);
    st->pos++;
    return c;
}

LexResult lex(char *buf, size_t buf_sz) {
    LexerState st = {.buf = buf, .size = buf_sz, .pos = 0};
    while (st.pos < st.size) {
        // TODO: process character
    }
}

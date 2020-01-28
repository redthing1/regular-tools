/*
asm.h
provides lexer and parser for the assembler
*/

#pragma once

#include <stdio.h>

typedef enum {
    UNKNOWN = 0,
    ALPHA = 1 << 0,   // abc
    NUMERIC = 1 << 1, // 123
    SPACE = 1 << 2,   // ' '
    ARGSEP = 1 << 3,  // ','
    MARK = 1 << 4,    // ':'
} CharType;

typedef struct {
    char *cont;
    CharType kind;
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

CharType classify_char(char c) {
    switch (c) {
    case ',':
        return ARGSEP;
    case ' ':
    case '\t':
        return SPACE;
    }
    // now categories
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        return ALPHA;
    } else if (c >= '0' && c <= '9') {
        return NUMERIC;
    }

    return UNKNOWN;
}

inline char peek_char(LexerState *st) { return st->buf[st->pos]; }

inline CharType peek_chartype(LexerState *st) { return classify_char(peek_char(st)); }

inline char take_char(LexerState *st) {
    char c = peek_char(st);
    st->pos++;
    return c;
}

void take_chars(LexerState *st, char *working, CharType readType) {
    while (!end_of_code() && (((int)peek_chartype(st) & (int)readType) > 0)) {
        char c = take_char(st);
        strcat(working, &c);
    }
}

void take_chars_until(LexerState *st, char *working, CharType stopType) {
    // CharType next_type = peek_chartype();
    while (st->pos < st->size && (((int)peek_chartype(st) & (int)stopType) == 0)) {
        char c = take_char(st);
        strcat(working, &c);
    }
}

void skip_chars(LexerState *st, CharType skip) {
    while (((int)peek_chartype(st) & (int)skip) > 0) {
        take_char(st);
    }
}

LexResult lex(char *buf, size_t buf_sz) {
    LexerState st = {.buf = buf, .size = buf_sz, .pos = 0};
    int token_buf_size = 128;
    Token *tokens = malloc(token_buf_size * sizeof(tokens));
    int token_count = 0;
    char *working = malloc(128);
    char cbuf[2] = {0, '\0'};

    while (st.pos < st.size) {
        if (token_count >= token_buf_size) {
            token_buf_size *= 2;
            tokens = realloc(tokens, token_buf_size * sizeof(tokens));
        }
        skip_chars(&st, SPACE);
        // process character
        char c = peek_char(&st);
        CharType c_type = classify_char(c);
        switch (c_type) {
        case ALPHA: { // start of identifier
            read_tokens(working, ALPHA);
            break;
        }
        case NUMERIC: { // start of num literal
            take_chars(&st, working, NUMERIC);
            Token tok = {.kind = NUMERIC, .cont = working};
            tokens[token_count++] = tok;
            break;
        }
        case ARGSEP: {
            take_chars(&st, working, ARGSEP);
            Token tok = {.kind = ARGSEP, .cont = working};
            tokens[token_count++] = tok;
            break;
        }
        case MARK: {
            take_chars(&st, working, MARK);
            Token tok = {.kind = MARK, .cont = working};
            tokens[token_count++] = tok;
            break;
        }
        }
    }
}

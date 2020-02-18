#pragma once

#include "util.h"
#include "buffie.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    UNKNOWN = 0,
    ALPHA = 1 << 0,            // abc
    NUMERIC = 1 << 1,          // 123
    SPACE = 1 << 2,            // ' '
    ARGSEP = 1 << 3,           // ','
    NUM_SPECIAL = 1 << 4,      // '$'
    MARK = 1 << 5,             // ':'
    QUOT = 1 << 6,             // '''
    BIND = 1 << 7,             // '@'
    DIRECTIVE_PREFIX = 1 << 8, // '#'
    NUMERIC_HEX = 1 << 9,      // beef
    PACK_START = 1 << 10,      // '\'
    IDENTIFIER = ALPHA | NUMERIC,
    DIRECTIVE = DIRECTIVE_PREFIX | ALPHA,
    NUMERIC_CONSTANT = NUMERIC | NUMERIC_HEX | NUM_SPECIAL,
} CharType;

typedef struct {
    char *cont;
    CharType kind;
} Token;

BUFFIE_OF(Token)

typedef struct {
    Buffie_Token tokens;
    int token_count;
} LexResult;

typedef struct {
    char *buf;
    size_t size;
    size_t pos;
    int line;
    int line_start;
} LexerState;

CharType classify_char(char c) {
    switch (c) {
    case ',':
        return ARGSEP;
    case ':':
        return MARK;
    case '\'':
        return QUOT;
    case '@':
        return BIND;
    case '#':
        return DIRECTIVE_PREFIX;
    case '\\':
        return PACK_START;
    case '$':
    case '.':
        return NUM_SPECIAL;
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return SPACE;
    }
    // now categories
    CharType type = UNKNOWN;
    if ((c >= 'a' && c <= 'f')) {
        type |= NUMERIC_HEX;
    }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
        type |= ALPHA;
    } else if (c >= '0' && c <= '9') {
        type |= NUMERIC;
    }

    return type;
}

char peek_char(LexerState *st) { return st->buf[st->pos]; }

CharType peek_chartype(LexerState *st) { return classify_char(peek_char(st)); }

char take_char(LexerState *st) {
    char c = peek_char(st);
    if (c == '\n') {
        st->line++;
        st->line_start = st->pos + 1;
    }
    st->pos++;
    return c;
}

void take_chars(LexerState *st, char *working, CharType readType) {
    while (st->pos < st->size && (((int)peek_chartype(st) & (int)readType) > 0)) {
        char c = take_char(st);
        strcat(working, &c);
    }
}

void take_chars_until(LexerState *st, char *working, CharType stopType) {
    while (st->pos < st->size && (((int)peek_chartype(st) & (int)stopType) == 0)) {
        char c = take_char(st);
        strcat(working, &c);
    }
}

void skip_chars(LexerState *st, CharType skip) {
    while (st->pos < st->size && ((int)peek_chartype(st) & (int)skip) > 0) {
        take_char(st);
    }
}

void skip_until(LexerState *st, char until) {
    while (st->pos < st->size && peek_char(st) != until) {
        take_char(st);
    }
}

Token make_token_of(LexerState *st, char *working, CharType type) {
    take_chars(st, working, type);
    Token tok = {.kind = type, .cont = working};
    return tok;
}

LexResult lex(char *buf, size_t buf_sz) {
    LexerState st = {.buf = buf, .size = buf_sz, .pos = 0, .line = 1, .line_start = 0};
    Buffie_Token tokens;
    buf_alloc_Token(&tokens, 256);
    char *working = NULL;
    size_t TOKEN_STR_SIZE = 128;

    while (st.pos < st.size) {
        skip_chars(&st, SPACE);         // skip any leading whitespace
        while (peek_char(&st) == ';') { // comments
            skip_until(&st, '\n');      // ignore the rest of the line
            skip_chars(&st, SPACE);     // skip any remaining space
        }
        if (st.pos > st.size - 1) { // check if end
            break;
        }
        // process character
        char c = peek_char(&st);
        working = util_strmk(TOKEN_STR_SIZE);

        CharType c_type = classify_char(c);
        if ((c_type & ALPHA) > 0) { // start of identifier
            buf_push_Token(&tokens, make_token_of(&st, working, IDENTIFIER));
        } else if ((c_type & NUMERIC) > 0) { // start of num literal
            buf_push_Token(&tokens, make_token_of(&st, working, NUMERIC));
        } else if ((c_type & ARGSEP) > 0) {
            buf_push_Token(&tokens, make_token_of(&st, working, ARGSEP));
        } else if ((c_type & MARK) > 0) {
            buf_push_Token(&tokens, make_token_of(&st, working, MARK));
        } else if ((c_type & QUOT) > 0) {
            buf_push_Token(&tokens, make_token_of(&st, working, QUOT));
        } else if ((c_type & BIND) > 0) {
            buf_push_Token(&tokens, make_token_of(&st, working, BIND));
        } else if ((c_type & NUM_SPECIAL) > 0) {
            buf_push_Token(&tokens, make_token_of(&st, working, NUMERIC_CONSTANT));
        } else if ((c_type & PACK_START) > 0) { // start of a pack, read in pack context
            buf_push_Token(&tokens, make_token_of(&st, working, PACK_START));

            working = util_strmk(TOKEN_STR_SIZE); // reallocate working

            CharType pack_escape = peek_chartype(&st);
            if (pack_escape == QUOT) { // \'
                buf_push_Token(&tokens, make_token_of(&st, working, QUOT));
            } else if (pack_escape == ALPHA) { // \x
                buf_push_Token(&tokens, make_token_of(&st, working, ALPHA));
            }
        } else if ((c_type & DIRECTIVE_PREFIX) > 0) {
            buf_push_Token(&tokens, make_token_of(&st, working, DIRECTIVE));
        } else {
            fprintf(stderr, "unrecognized character: %c, [%d:%d]\n", c, st.line, (int)(st.pos - st.line_start) + 1);
            take_char(&st); // eat the character
        }
    }
    LexResult res;
    res.tokens = tokens;
    res.token_count = tokens.ct;
    return res;
}

void free_lex_result(LexResult lexed) {
    // free all tokens
    for (int i = 0; i < lexed.token_count; i++) {
        Token tok = buf_get_Token(&lexed.tokens, i);
        free(tok.cont);
    }
    // free token buffie
    buf_free_Token(&lexed.tokens);
}
/*
asm.h
provides lexer and parser for the assembler
*/

#pragma once

#include "instr.h"
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
    MARK = 1 << 4,             // ':'
    NUM_SPECIAL = 1 << 5,      // '$'
    DIRECTIVE_PREFIX = 1 << 6, // '#'
    IDENTIFIER = ALPHA | NUMERIC,
    DIRECTIVE = DIRECTIVE_PREFIX | ALPHA,
    NUMERIC_CONSTANT = NUMERIC | NUM_SPECIAL,
} CharType;

typedef struct {
    char *cont;
    CharType kind;
} Token;

/* #region Lexer */

typedef struct {
    Token *tokens;
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
    case '#':
        return DIRECTIVE_PREFIX;
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
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        return ALPHA;
    } else if (c >= '0' && c <= '9') {
        return NUMERIC;
    }

    return UNKNOWN;
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

LexResult lex(char *buf, size_t buf_sz) {
    LexerState st = {.buf = buf, .size = buf_sz, .pos = 0, .line = 1, .line_start = 0};
    int token_buf_size = 128;
    Token *tokens = malloc(token_buf_size * sizeof(tokens));
    int token_count = 0;
    char *working = NULL;

    while (st.pos < st.size) {
        if (token_count >= token_buf_size) {
            token_buf_size *= 2;
            tokens = realloc(tokens, token_buf_size * sizeof(tokens));
        }
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
        working = malloc(128);
        working[0] = '\0';
        CharType c_type = classify_char(c);
        switch (c_type) {
        case ALPHA: { // start of identifier
            take_chars(&st, working, IDENTIFIER);
            Token tok = {.kind = IDENTIFIER, .cont = working};
            tokens[token_count++] = tok;
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
        case NUM_SPECIAL: {
            take_chars(&st, working, NUMERIC_CONSTANT);
            Token tok = {.kind = NUMERIC_CONSTANT, .cont = working};
            tokens[token_count++] = tok;
            break;
        }
        case DIRECTIVE_PREFIX: {
            take_chars(&st, working, DIRECTIVE);
            Token tok = {.kind = DIRECTIVE, .cont = working};
            tokens[token_count++] = tok;
            break;
        }
        default: {
            fprintf(stderr, "unrecognized character: %c, [%d:%d]\n", c, st.line, (int)(st.pos - st.line_start) + 1);
            take_char(&st); // eat the character
            break;
        }
        }
    }
    LexResult res;
    res.tokens = tokens;
    res.token_count = token_count;
    return res;
}

void free_lex_result(LexResult lexed) {
    // free all tokens
    for (int i = 0; i < lexed.token_count; i++) {
        free(lexed.tokens[i].cont);
    }
    // free token array
    free(lexed.tokens);
}

/* #endregion */

/* #region Parser */

typedef struct {
    Statement *statements;
    int statement_count;
    int entry;
    int status;
} Program;

typedef struct {
    LexResult *lexed;
    int token; // token index
    int cpos;  // char position of reading
} ParserState;

Token peek_token(ParserState *st) { 
    if (st->token > st->lexed->token_count - 1) {
        return (Token) {.kind = UNKNOWN};
    }
    return st->lexed->tokens[st->token];
}

Token take_token(ParserState *st) {
    Token tok = peek_token(st);
    st->token++;
    st->cpos += strlen(tok.cont);
    return tok;
}

Token expect_token(ParserState *st, CharType type) {
    Token next = peek_token(st);
    CharType next_type = next.kind;
    if (next_type == type) {
        // expected token found
        return take_token(st);
    } else {
        // expected token not found
        printf("unexpected token@%d: %s [%d]\n", st->cpos, next.cont, next.kind);
        return (Token){.cont = NULL, .kind = UNKNOWN};
    }
}

uint32_t parse_numeric(ParserState *st) {
    // interpret numeric token
    Token num_tok = expect_token(st, NUMERIC_CONSTANT);
    char pfx = num_tok.cont[0];
    // create a new string without the prefix
    int num_len = strlen(num_tok.cont) - 1;
    char *num_str = malloc(num_len + 1);
    strncpy(num_str, num_tok.cont + 1, num_len);
    num_str[num_len] = '\0';
    // convert base
    uint32_t val = 0;
    switch (pfx) {
    case '$': {
        // interpret as base-16
        val = (int)strtol(num_str, NULL, 16);
        break;
    }
    case '.': {
        // interpret as base-10
        val = atoi(num_str);
        break;
    }
    default:
        // invalid numeric
        printf("ERR: invalid numeric prefix %c", pfx);
    }
    free(num_str); // free numstr
    return val;
}

void populate_statement(Statement *stmt) {
        const char *mnem = get_instruction_mnem(stmt->opcode);
        InstructionInfo instr_info = get_instruction_info((char*) mnem);
        stmt->type = instr_info.type;
}

Statement parse_statement(ParserState *st, char *mnem) {
    InstructionInfo instr_info = get_instruction_info(mnem);
    Statement stmt = {.opcode = 0, .a1 = 0, .a2 = 0, .a3 = 0, .type = instr_info.type};
    if (instr_info.type == INSTR_INV) {
        // invalid mnemonic
        printf("unrecognized mnemonic: %s\n", mnem);
        return stmt; // an invalid instruction statement
    }

    stmt.opcode = instr_info.opcode; // set opcode
    // read the instruction data
    Token t1, t2, t3;
    if ((instr_info.type & INSTR_K_R1) > 0) {
        t1 = expect_token(st, IDENTIFIER);
        stmt.a1 = get_register(t1.cont);
    }
    if ((instr_info.type & INSTR_K_R2) > 0) {
        t2 = expect_token(st, IDENTIFIER);
        stmt.a2 = get_register(t2.cont);
    }
    if ((instr_info.type & INSTR_K_R3) > 0) {
        t3 = expect_token(st, IDENTIFIER);
        stmt.a3 = get_register(t3.cont);
    }
    if ((instr_info.type & INSTR_K_I1) > 0) {
        // 24-bit constant
        uint32_t val = parse_numeric(st);
        stmt.a1 = (ARG)(val >> 16);        // upper 8
        stmt.a2 = (ARG)((val >> 8) & 255); // middle 8
        stmt.a3 = (ARG)(val & 255);        // lower 8
    } else if ((instr_info.type & INSTR_K_I2) > 0) {
        // 16-bit constant
        uint32_t val = parse_numeric(st);
        stmt.a2 = (ARG)(val >> 8);  // upper 8
        stmt.a3 = (ARG)(val & 255); // lower 8
    } else if ((instr_info.type & INSTR_K_I3) > 0) {
        // 8-bit constant
        uint32_t val = parse_numeric(st);
        stmt.a3 = (ARG)val;
    }
    return stmt;
}

Program parse(LexResult lexed) {
    ParserState st = {.lexed = &lexed, .token = 0, .cpos = 0};
    int statement_buf_size = 128;
    Statement *statements = malloc(statement_buf_size * sizeof(statements));
    int statement_count = 0;
    Program prg = {.statements = statements, .status = 0};

    // parse the lex result into a list of instructions
    while (st.token < lexed.token_count) {
        // TODO: check to reallocate statements
        if (statement_count > statement_buf_size - 1) {
            printf("ERROR: out of statement space\n");
            return prg;
        }
        Token next = peek_token(&st);
        switch (next.kind) {
        case DIRECTIVE: { // handle directive
            Token tok = take_token(&st);
            // check if it is the "#entry" directive
            if (streq(tok.cont, "#entry")) {
                // following label has the entry point
                expect_token(&st, MARK);
                Token lbl = expect_token(&st, IDENTIFIER);
                // TODO: interpret the entry point label
                printf("TODO: @#%d store entrypoint\n", st.token);
            }
            break;
        }
        case IDENTIFIER: {
            Token id_token = take_token(&st);
            Token id_next = peek_token(&st);
            switch (id_next.kind) {
            case MARK: {
                // this is a label definition ("label:")
                take_token(&st); // eat the mark
                // TODO: store the label
                printf("TODO: @#%d store label\n", st.token);
                break;
            }
            
            default: {
                // this is an instruction
                // look at the instruction name and figure out what to do
                char *mnem = id_token.cont;
                Statement stmt = parse_statement(&st, mnem);
                // dump instruction info
                statements[statement_count++] = stmt;
                break;
            }
            }
            break;
        }
        default:
            printf("ERR: unexpected token #%d\n", st.token);
            prg.status = 1;
            return prg;
        }
    }

    // update program information
    prg.statement_count = statement_count;
    return prg;
}

void free_program(Program prg) {
    // free statement array
    free(prg.statements);
}

/* #endregion */

/* #region Binary */

void write_program(FILE *ouf, Program prg) {
    char w = '\0';
    // write header
    // TODO: header format
    // write code
    for (int i = 0; i < prg.statement_count; i++) {
        Statement st = prg.statements[i];
        // write binary opcode
        w = st.opcode;
        fwrite(&w, 1, sizeof(w), ouf);
        // write binary args
        w = st.a1;
        fwrite(&w, 1, sizeof(w), ouf);
        w = st.a2;
        fwrite(&w, 1, sizeof(w), ouf);
        w = st.a3;
        fwrite(&w, 1, sizeof(w), ouf);
    }
}

/* #endregion */

/* #region Debugging */

void dump_statement(Statement st) {
    const char *op_name = get_instruction_mnem(st.opcode);
    printf("OP: [%3s]", op_name);
    if ((st.type & INSTR_K_R1) > 0) {
        printf(" %-3s", get_register_name(st.a1));
    }
    if ((st.type & INSTR_K_R2) > 0) {
        printf(" %-3s", get_register_name(st.a2));
    }
    if ((st.type & INSTR_K_R3) > 0) {
        printf(" %-3s", get_register_name(st.a3));
    }
    if ((st.type & INSTR_K_I1) > 0) {
        uint32_t v = (st.a1 << 16) | (st.a2 << 8) | st.a3;
        printf(" $%02x", v);
    } else if ((st.type & INSTR_K_I2) > 0) {
        uint32_t v = (st.a2 << 8) | st.a3;
        printf(" $%02x", v);
    } else if ((st.type & INSTR_K_I3) > 0) {
        printf(" $%02x", st.a3);
    }
    printf("\n");
}

void dump_program(Program prg) {
    for (int i = 0; i < prg.statement_count; i++) {
        Statement st = prg.statements[i];
        dump_statement(st);
    }
}

/* #endregion */

/*
asm.h
provides lexer and parser for the assembler
*/

#pragma once

#include "ds.h"
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
    NUMERIC_HEX = 1 << 7,      // beef
    PACK_START = 1 << 8,       // '\'
    IDENTIFIER = ALPHA | NUMERIC,
    DIRECTIVE = DIRECTIVE_PREFIX | ALPHA,
    NUMERIC_CONSTANT = NUMERIC | NUMERIC_HEX | NUM_SPECIAL,
    PACK = NUMERIC | NUMERIC_HEX,
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

LexResult lex(char *buf, size_t buf_sz) {
    LexerState st = {.buf = buf, .size = buf_sz, .pos = 0, .line = 1, .line_start = 0};
    int token_buf_size = 256;
    Token *tokens = malloc(token_buf_size * sizeof(Token));
    int token_count = 0;
    char *working = NULL;

    while (st.pos < st.size) {
        if (token_count >= token_buf_size) {
            token_buf_size *= 2;
            tokens = realloc(tokens, token_buf_size * sizeof(Token));
            printf("reallocating tokens[]\n");
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
        if ((c_type & ALPHA) > 0) { // start of identifier
            take_chars(&st, working, IDENTIFIER);
            Token tok = {.kind = IDENTIFIER, .cont = working};
            tokens[token_count++] = tok;
        } else if ((c_type & NUMERIC) > 0) { // start of num literal
            take_chars(&st, working, NUMERIC);
            Token tok = {.kind = NUMERIC, .cont = working};
            tokens[token_count++] = tok;
        } else if ((c_type & ARGSEP) > 0) {
            take_chars(&st, working, ARGSEP);
            Token tok = {.kind = ARGSEP, .cont = working};
            tokens[token_count++] = tok;
        } else if ((c_type & MARK) > 0) {
            take_chars(&st, working, MARK);
            Token tok = {.kind = MARK, .cont = working};
            tokens[token_count++] = tok;
        } else if ((c_type & NUM_SPECIAL) > 0) {
            take_chars(&st, working, NUMERIC_CONSTANT);
            Token tok = {.kind = NUMERIC_CONSTANT, .cont = working};
            tokens[token_count++] = tok;
        } else if ((c_type & PACK_START) > 0) {
            take_char(&st); // eat pack start
            // hex data after start indicator
            take_chars(&st, working, PACK);
            Token tok = {.kind = PACK, .cont = working};
            tokens[token_count++] = tok;
        } else if ((c_type & DIRECTIVE_PREFIX) > 0) {
            take_chars(&st, working, DIRECTIVE);
            Token tok = {.kind = DIRECTIVE, .cont = working};
            tokens[token_count++] = tok;
        } else {
            fprintf(stderr, "unrecognized character: %c, [%d:%d]\n", c, st.line, (int)(st.pos - st.line_start) + 1);
            take_char(&st); // eat the character
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
    uint16_t entry;
    int status;
    BYTE *data;
    uint16_t data_size;
} Program;

typedef struct {
    LexResult *lexed;
    int token;    // token index
    int cpos;     // char position of reading
    int offset;   // binary output position
    List *labels; // label list
} ParserState;

typedef struct {
    char *label;
    int offset;
} Label;

void program_init(Program *p) {
    p->statements = NULL;
    p->statement_count = 0;
    p->entry = 0;
    p->status = 0;
    p->data = NULL;
    p->data_size = 0;
}

void parser_state_cleanup(ParserState *st) {
    // clean up labels
    while (!list_empty(st->labels)) {
        Label* lb = (Label*) list_pop(st->labels); // pop off remaining labels
        free(lb); // free label
    }
}

Token peek_token(ParserState *st) {
    if (st->token > st->lexed->token_count - 1) {
        return (Token){.kind = UNKNOWN};
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
        printf("unexpected token#%d @%d: %s [%d]\n", st->token, st->cpos, next.cont, next.kind);
        return (Token){.cont = NULL, .kind = UNKNOWN};
    }
}

void parse_label_ref(ParserState *st, Token mark, char *prev) {
    // check if it's a double tok
    if (strlen(mark.cont) > 1) { // check if :: instead of :
        // this is a label reference ("::label")
        Token lbref = peek_token(st);
        // find the matching label and replace with label offset
        ListNode *n = st->labels->top;
        uint16_t lb_offset = 0;
        do {
            Label *lb = (Label *)(n->data);
            if (streq(lb->label, lbref.cont)) {
                lb_offset = lb->offset;
                break;
            }
            n = n->link;
        } while (n);
        // hotpatch the token (free first)
        free(lbref.cont);
        char *patch_cbuf = malloc(128);
        patch_cbuf[0] = '\0';
        sprintf(patch_cbuf, ".%d", lb_offset);

        st->lexed->tokens[st->token].cont = patch_cbuf;
        st->lexed->tokens[st->token].kind = NUMERIC_CONSTANT;
    } else {
        // this is a label definition ("label:")
        // store the label
        // create and store a label
        Label *label = malloc(sizeof(Label));
        label->label = prev;
        label->offset = st->offset;
        list_push(st->labels, label);
    }
}

uint32_t parse_numeric(ParserState *st) {
    // look at the next token (it could be a ref)
    Token next_tok = peek_token(st);
    if (next_tok.kind == MARK) {
        Token mark = expect_token(st, MARK); // eat the mark
        parse_label_ref(st, mark, NULL);
    }
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
    InstructionInfo instr_info = get_instruction_info((char *)mnem);
    stmt->type = instr_info.type;
    stmt->sz = instr_info.fin_sz;
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
    stmt.sz = instr_info.fin_sz;     // set instruction size

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
        stmt.a1 = (ARG)(val & 0xff);        // lower 8
        stmt.a2 = (ARG)((val >> 8) & 0xff); // middle 8
        stmt.a3 = (ARG)(val >> 16);         // upper 8
    } else if ((instr_info.type & INSTR_K_I2) > 0) {
        // 16-bit constant
        uint32_t val = parse_numeric(st);
        stmt.a2 = (ARG)(val & 0xff); // lower 8
        stmt.a3 = (ARG)(val >> 8);   // upper 8
    } else if ((instr_info.type & INSTR_K_I3) > 0) {
        // 8-bit constant
        uint32_t val = parse_numeric(st);
        stmt.a3 = (ARG)val;
    }
    return stmt;
}

Program parse(LexResult lexed) {
    ParserState st = {.lexed = &lexed, .token = 0, .cpos = 0, .offset = 0};
    int statement_buf_size = 128;
    Statement *statements = malloc(statement_buf_size * sizeof(Statement));
    int statement_count = 0;

    Program prg;
    program_init(&prg);
    prg.statements = statements;

    List labels;
    st.labels = &labels;
    list_init(&labels);
    char *entry_lbl = NULL;

    // parse the lex result into a list of instructions
    while (st.token < lexed.token_count) {
        // TODO: check to reallocate statements
        if (statement_count > statement_buf_size - 1) {
            statement_buf_size *= 2;
            statements = realloc(statements, statement_buf_size * sizeof(Statement));
            printf("reallocating statements[]\n");
            return prg;
        }
        Token next = peek_token(&st);
        switch (next.kind) {
        case DIRECTIVE: { // handle directive
            Token dir = take_token(&st);
            // check if it is the "#entry" directive
            if (streq(dir.cont, "#entry")) {
                // following label has the entry point
                expect_token(&st, MARK);
                Token lbl = expect_token(&st, IDENTIFIER);
                // store entry label
                entry_lbl = lbl.cont;
            } else if (streq(dir.cont, "#d")) { // data directive
                Token pack = expect_token(&st, PACK);
                size_t pack_len = strlen(pack.cont);
                if (pack_len % 2 != 0) {
                    // odd number of half-bytes, invalid
                    printf("ERROR: invalid data (must be even)");
                }
                pack_len = pack_len / 2; // divide by two because 0xff = 1 byte
                BYTE *pack_data = datahex(pack.cont); // convert data from hex
                reverse_bytes(pack_data, pack_len); // flip data (for endianness)
                // write the pack data to the binary
                if (!prg.data) {
                    prg.data = malloc(sizeof(BYTE) * pack_len);
                } else {
                    prg.data = realloc(prg.data, sizeof(BYTE) * (prg.data_size + pack_len));
                }
                // copy the data
                memcpy(prg.data + prg.data_size, pack_data, pack_len);
                free(pack_data); // free decoded data
                // update offset
                prg.data_size += pack_len;
                st.offset += pack_len;
            }
            break;
        }
        case IDENTIFIER: {
            Token id_token = take_token(&st);
            Token id_next = peek_token(&st);
            bool instr = true;
            if (id_next.kind == MARK) {
                Token mark = expect_token(&st, MARK); // eat the mark
                if (strlen(mark.cont) == 1) {         // if single :, then label def
                    instr = false;
                }
                parse_label_ref(&st, mark, id_token.cont);
            }
            if (instr) {
                // this is an instruction
                // look at the instruction name and figure out what to do
                char *mnem = id_token.cont;
                Statement stmt = parse_statement(&st, mnem);
                // dump instruction info
                statements[statement_count++] = stmt;
                // update offset
                st.offset += stmt.sz;
            }
            break;
        }
        default:
            printf("ERR: unexpected token #%d\n", st.token);
            prg.status = 1;
            parser_state_cleanup(&st);
            return prg;
        }
    }

    // check for entry point label
    if (entry_lbl) {
        // find the matching label and replace with label offset
        ListNode *n = labels.top;
        uint16_t lb_offset = 0;
        do {
            Label *lb = (Label *)(n->data);
            if (streq(lb->label, entry_lbl)) {
                lb_offset = lb->offset;
                break;
            }
            n = n->link;
        } while (n);
        // set entrypoint to label offset
        prg.entry = lb_offset;
        // add data size to entry offset
        prg.entry += prg.data_size;
    }

    parser_state_cleanup(&st);

    // update program information
    prg.statement_count = statement_count;
    return prg;
}

void free_program(Program prg, bool free_data) {
    // free statement array
    free(prg.statements);
    if (free_data && prg.data) {
        // free data
        free(prg.data);
    }
}

/* #endregion */

/* #region Binary */

#define HEADER_SIZE 8

void write_program(FILE *ouf, Program prg, bool write_header) {
    char w = '\0';
    if (write_header) {
        // write header
        const char *REG = "rg";
        fputs(REG, ouf);                               // magic
        fwrite(&prg.entry, sizeof(prg.entry), 1, ouf); // entrypoint
        uint16_t code_size = prg.statement_count * INSTR_SIZE;
        fwrite(&code_size, sizeof(code_size), 1, ouf);         // code size
        fwrite(&prg.data_size, sizeof(prg.data_size), 1, ouf); // data size
        printf("header[%d] \n", HEADER_SIZE);
    } else {
        printf("--bare given, not writing header\n");
    }

    // write data
    if (prg.data) {
        for (int i = 0; i < prg.data_size; i++) {
            w = prg.data[i];
            fwrite(&w, sizeof(w), 1, ouf);
        }
        printf("data[%d] \n", (int)prg.data_size);
    }

    // write code
    size_t code_offset = 0;
    for (int i = 0; i < prg.statement_count; i++) {
        Statement st = prg.statements[i];
        // write binary opcode
        w = st.opcode;
        fwrite(&w, sizeof(w), 1, ouf);
        // write binary args
        w = st.a1;
        fwrite(&w, sizeof(w), 1, ouf);
        w = st.a2;
        fwrite(&w, sizeof(w), 1, ouf);
        w = st.a3;
        fwrite(&w, sizeof(w), 1, ouf);
        code_offset += st.sz;
    }
    printf("code[%d] \n", (int)code_offset);
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
        uint32_t v = st.a1 | (st.a2 << 8) | (st.a3 << 16);
        printf(" $%04x", v);
    } else if ((st.type & INSTR_K_I2) > 0) {
        uint32_t v = st.a2 | (st.a3 << 8);
        printf(" $%04x", v);
    } else if ((st.type & INSTR_K_I3) > 0) {
        printf(" $%04x", st.a3);
    }
    printf("\n");
}

void dump_program(Program prg) {
    printf("entry:     $%04x\n", prg.entry);
    uint16_t code_size = prg.statement_count * INSTR_SIZE;
    printf("code size: $%04x\n", code_size);
    printf("data size: $%04x\n", prg.data_size);
    int offset = HEADER_SIZE + prg.data_size;
    for (int i = 0; i < prg.statement_count; i++) {
        Statement st = prg.statements[i];
        printf("%04x ", offset);
        dump_statement(st);
        offset += st.sz;
    }
}

/* #endregion */

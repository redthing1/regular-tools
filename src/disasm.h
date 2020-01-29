/*
disasm.h
provides dissasembly
*/

#pragma once

#include "asm.h"

typedef struct {
    char *buf;
    size_t size;
    size_t pos;
} DecoderState;

ARG take_arg(DecoderState *st) { return st->buf[st->pos++]; }

Program decode_program(char *buf, size_t buf_sz) {
    DecoderState st = {.buf = buf, .size = buf_sz, .pos = 0};
    int statement_buf_size = 128;
    Statement *statements = malloc(statement_buf_size * sizeof(statements));
    int statement_count = 0;
    Program prg = {.statements = statements, .status = 0};
    // check size multiple
    if ((buf_sz % 4) != 0) {
        // invalid size for program
        printf("invalid size %d for program.\n", (int)buf_sz);
        prg.status = 1;
        return prg;
    }

    while (st.pos < buf_sz) {

        ARG op = take_arg(&st);
        ARG a1 = take_arg(&st);
        ARG a2 = take_arg(&st);
        ARG a3 = take_arg(&st);
        
        // interpret instruction
        Statement stmt = {.opcode = op, .a1 = a1, .a2 = a2, .a3 = a3};
        populate_statement(&stmt);

        // save statement
        statements[statement_count++] = stmt;
    }

    prg.statement_count = statement_count;
    return prg;
}

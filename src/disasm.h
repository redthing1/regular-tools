/*
disasm.h
provides dissasembly
*/

#pragma once

#include "asm.h"
#include "instr.h"

typedef struct {
    char *buf;
    size_t size;
    size_t pos;
} DecoderState;

ARG take_arg(DecoderState *st) { return st->buf[st->pos++]; }

Program decode_program(char *buf, size_t buf_sz) {
    // read header
    bool valid_magic = (buf[0] == 'r') && (buf[1] == 'e') && (buf[2] == 'g');
    uint16_t entry;
    uint16_t code_size;
    size_t dec_offset = 0;
    if (valid_magic) {
        entry = (buf[6] << 8) | buf[7];
        code_size = (buf[8] << 8) | buf[9];
        dec_offset = 10; // start after header
    } else {
        printf("WARN: magic header not matched. falling back to compat.\n");
        // set default values
        entry = 0;
        code_size = buf_sz;
    }

    DecoderState st = {.buf = buf, .size = code_size, .pos = dec_offset};
    int statement_buf_size = 128;
    Statement *statements = malloc(statement_buf_size * sizeof(statements));
    int statement_count = 0;
    Program prg = {.statements = statements, .status = 0, .entry = entry};
    // check size multiple
    if ((code_size % INSTR_SIZE) != 0) {
        // invalid size for program
        printf("invalid size %d for program.\n", (int)code_size);
        prg.status = 1;
        return prg;
    }

    while (st.pos < code_size) {

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

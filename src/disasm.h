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

typedef struct {
    bool valid_magic;
    // uint16_t entry;
    uint16_t code_size;
    uint16_t data_size;
    size_t decode_offset;
} RGHeader;

ARG take_arg(DecoderState *st) { return st->buf[st->pos++]; }

RGHeader decode_header(char *buf, size_t buf_sz) {
    RGHeader hd;
    hd.valid_magic = (buf[0] == 'r') && (buf[1] == 'g');
    if (hd.valid_magic) {
        uint8_t code_size_l = buf[2];
        uint8_t code_size_h = buf[3];
        hd.code_size = code_size_l | (code_size_h << 8);

        uint8_t data_size_l = buf[4];
        uint8_t data_size_h = buf[5];
        hd.data_size = data_size_l | (data_size_h << 8);

        hd.decode_offset = HEADER_SIZE; // start after header
    } else {
        printf("WARN: magic header not matched. reading as bare binary.\n");
        // set default values
        hd.code_size = buf_sz;
        hd.data_size = 0;
        hd.decode_offset = 0;
    }
    return hd;
}

void dump_header(RGHeader hd) {
    // printf("entry:     $%04x\n", hd.entry);
    printf("code size: $%04x\n", hd.code_size);
    printf("data size: $%04x\n", hd.data_size);
}

typedef struct {
    CompiledProgram cmp;
    int status;
} DecoderResult;

DecoderResult decode_compiled_program(char *buf, size_t buf_sz) {
    // read header
    RGHeader hd = decode_header(buf, buf_sz);
    DecoderState st = {.buf = buf, .size = hd.code_size, .pos = hd.decode_offset + hd.data_size};

    CompiledProgram cmp;
    compiled_program_init(&cmp);
    cmp.data_size = hd.data_size;
    cmp.instructions = malloc(sizeof(Instruction) * cmp.instruction_count);

    // check size multiple
    if ((hd.code_size % INSTR_SIZE) != 0) {
        // invalid size for program
        printf("invalid size %d for program.\n", (int)hd.code_size);
        DecoderResult res;
        res.cmp = cmp;
        res.status = 1;
        return res;
    }

    size_t code_end = hd.decode_offset + hd.data_size + hd.code_size;
    while (st.pos < code_end) {
        ARG op = take_arg(&st);
        ARG a1 = take_arg(&st);
        ARG a2 = take_arg(&st);
        ARG a3 = take_arg(&st);

        // interpret instruction
        Instruction in = {.opcode = op, .a1 = a1, .a2 = a2, .a3 = a3};
        cmp.instructions[cmp.instruction_count++] = in; // save instruction
    }

    DecoderResult res;
    res.status = 0;
    res.cmp = cmp;
    return res;
}

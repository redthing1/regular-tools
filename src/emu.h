/*
disasm.h
provides emulation capability
*/

#pragma once
#include "disasm.h"
#include "instr.h"
#include <stdbool.h>

const size_t MEMORY_SIZE = 64 * 1024; // 65K
const size_t REGISTER_COUNT = 32;

typedef struct {
    ARG *reg;
    BYTE *mem;
    size_t mem_sz;
    bool executing;
} EmulatorState;

EmulatorState *emu_init() {
    EmulatorState *emu_st = malloc(sizeof(EmulatorState));
    emu_st->mem_sz = MEMORY_SIZE;
    emu_st->mem = malloc(MEMORY_SIZE * sizeof(BYTE));
    emu_st->reg = malloc(REGISTER_COUNT * sizeof(ARG));

    return emu_st;
}

/**
 * Load the program data into memory
 */
RGHeader emu_load(EmulatorState *emu_st, int offset, char *program, size_t program_sz) {
    // read RG header
    RGHeader hd = decode_header(program, program_sz);
    dump_header(hd);
    // offset the copy to start after the header
    size_t copy_sz = program_sz - hd.decode_offset;
    memcpy(emu_st->mem + offset, program + hd.decode_offset, copy_sz);
    return hd;
}

void emu_exec(EmulatorState *emu_st, Statement instr) {
    switch (instr.opcode) {
    case OP_NOP: {
        // do nothing
        break;
    }
    case OP_ADD: {
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2] + emu_st->reg[instr.a3];
        break;
    }
    case OP_SUB: {
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2] + emu_st->reg[instr.a3];
        break;
    }
    case OP_SET: {
        emu_st->reg[instr.a1] = instr.a2 | (instr.a3 << 8);
        break;
    }
    case OP_MOV: {
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2];
        break;
    }
    case OP_HLT: {
        emu_st->executing = false;
        break;
    }
    default:
        break;
    }
}

void emu_run(EmulatorState *emu_st, ARG entry) {
    // set PC regiemu_ster to entrypoint
    emu_st->reg[0] = entry;
    emu_st->executing = true;
    // emu_start decode loop
    while (emu_st->executing) {
        // decode inemu_struction
        ARG op = emu_st->mem[emu_st->reg[0]];
        ARG d1 = emu_st->mem[emu_st->reg[0] + 1];
        ARG d2 = emu_st->mem[emu_st->reg[0] + 2];
        ARG d3 = emu_st->mem[emu_st->reg[0] + 3];
        emu_st->reg[0] += INSTR_SIZE; // advance PC
        Statement stmt = {.opcode = op, .a1 = d1, .a2 = d2, .a3 = d3 };
        populate_statement(&stmt); // interpret instruction type
        dump_statement(stmt);
        emu_exec(emu_st, stmt);
    }
}

void emu_free(EmulatorState *emu_st) {
    // free data
    free(emu_st->reg);
    free(emu_st->mem);
    // free emu emu_state
    free(emu_st);
}

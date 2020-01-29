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
const size_t SIMPLE_REGISTER_COUNT = 8;

typedef struct {
    WORD *reg;
    BYTE *mem;
    size_t mem_sz;
    bool executing;
} EmulatorState;

EmulatorState *emu_init() {
    EmulatorState *emu_st = malloc(sizeof(EmulatorState));
    emu_st->mem_sz = MEMORY_SIZE;

    size_t mem_alloc_sz = MEMORY_SIZE * sizeof(BYTE);
    emu_st->mem = malloc(mem_alloc_sz);
    size_t reg_alloc_sz = REGISTER_COUNT * sizeof(WORD);
    emu_st->reg = malloc(reg_alloc_sz);

    // initialize all data
    memset(emu_st->mem, 0, mem_alloc_sz);
    memset(emu_st->reg, 0, reg_alloc_sz);

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

void dump_rg(EmulatorState *emu_st, ARG rg) {
    const char *reg_name = get_register_name(rg);
    printf("%5s: $%08x\n", reg_name, emu_st->reg[rg]);
}

/**
 * Dump the emulator state for debugging
 */
void emu_dump(EmulatorState *emu_st) {
    printf("== STATE ==\n");
    // dump main registers
    for (ARG i = 0; i < SIMPLE_REGISTER_COUNT; i++) {
        dump_rg(emu_st, i);
    }
    // dump special registers
    dump_rg(emu_st, REG_RSP);
    dump_rg(emu_st, REG_RAT);
}

/**
 * Execute an instruction in the emulator
 */
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
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2] - emu_st->reg[instr.a3];
        break;
    }
    case OP_AND: {
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2] & emu_st->reg[instr.a3];
        break;
    }
    case OP_ORR: {
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2] | emu_st->reg[instr.a3];
        break;
    }
    case OP_XOR: {
        emu_st->reg[instr.a1] = emu_st->reg[instr.a2] ^ emu_st->reg[instr.a3];
        break;
    }
    case OP_NOT: {
        emu_st->reg[instr.a1] = ~emu_st->reg[instr.a2];
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
    case OP_LDW: {
        UWORD addr = emu_st->reg[instr.a2];
        emu_st->reg[instr.a1] = emu_st->mem[addr + 0] << 0 | emu_st->mem[addr + 1] << 8 | emu_st->mem[addr + 2] << 16 |
                                emu_st->mem[addr + 3] << 24;
        break;
    }
    case OP_STW: {
        UWORD addr = emu_st->reg[instr.a2];
        emu_st->mem[addr + 0] = (emu_st->reg[instr.a1] >> 0) & 0xff;
        emu_st->mem[addr + 1] = (emu_st->reg[instr.a1] >> 8) & 0xff;
        emu_st->mem[addr + 2] = (emu_st->reg[instr.a1] >> 16) & 0xff;
        emu_st->mem[addr + 3] = (emu_st->reg[instr.a1] >> 24) & 0xff;
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

/**
 * Start emulator execution at an entry point in memory
 */
void emu_run(EmulatorState *emu_st, ARG entry) {
    // set PC regiemu_ster to entrypoint
    emu_st->reg[REG_RPC] = entry;
    emu_st->executing = true;
    // emu_start decode loop
    while (emu_st->executing) {
        // decode inemu_struction
        ARG op = emu_st->mem[emu_st->reg[0]];
        ARG d1 = emu_st->mem[emu_st->reg[0] + 1];
        ARG d2 = emu_st->mem[emu_st->reg[0] + 2];
        ARG d3 = emu_st->mem[emu_st->reg[0] + 3];
        emu_st->reg[0] += INSTR_SIZE; // advance PC
        Statement stmt = {.opcode = op, .a1 = d1, .a2 = d2, .a3 = d3};
        populate_statement(&stmt); // interpret instruction type

        dump_statement(stmt);   // dump statement
        emu_exec(emu_st, stmt); // execute statement
        emu_dump(emu_st);       // dump state
    }
}

void emu_free(EmulatorState *emu_st) {
    // free data
    free(emu_st->reg);
    free(emu_st->mem);
    // free emu emu_state
    free(emu_st);
}

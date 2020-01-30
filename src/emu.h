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

#define INTERRUPT_PAUSE 0x01   // pause execution
#define INTERRUPT_DUMPCPU 0x02 // dump cpu state
#define INTERRUPT_DUMPMEM 0x03 // dump memory page
#define INTERRUPT_DUMPSTK 0x04 // dump stack

typedef struct {
    UWORD *reg;
    BYTE *mem;
    size_t mem_sz;
    bool executing;
    uint64_t ticks;
    bool debug;
    bool onestep; // step one at a time
} EmulatorState;

/* #region Init, Deinit, and Loading */

EmulatorState *emu_init() {
    EmulatorState *emu_st = malloc(sizeof(EmulatorState));
    emu_st->mem_sz = MEMORY_SIZE;

    size_t mem_alloc_sz = MEMORY_SIZE * sizeof(BYTE);
    emu_st->mem = malloc(mem_alloc_sz);
    size_t reg_alloc_sz = REGISTER_COUNT * sizeof(UWORD);
    emu_st->reg = malloc(reg_alloc_sz);

    // initialize all data
    memset(emu_st->mem, 0, mem_alloc_sz);
    memset(emu_st->reg, 0, reg_alloc_sz);

    // set RSP to last word
    emu_st->reg[REG_RSP] = emu_st->mem_sz - sizeof(WORD);

    // reset settings
    emu_st->debug = false;
    emu_st->onestep = 0;
    emu_st->ticks = 0;

    return emu_st;
}

void emu_free(EmulatorState *emu_st) {
    // free data
    free(emu_st->reg);
    free(emu_st->mem);
    // free emu emu_state
    free(emu_st);
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

/* #endregion */

/* #region Dumping */

void dump_rg(EmulatorState *emu_st, ARG rg) {
    const char *reg_name = get_register_name(rg);
    printf("%5s: $%08x\n", reg_name, emu_st->reg[rg]);
}

/**
 * Dump the emulator state for debugging
 */
void emu_dump(EmulatorState *emu_st, bool full) {
    printf("== STATE ==\n");
    size_t dump_regs = SIMPLE_REGISTER_COUNT;
    if (full) {
        dump_regs = REGISTER_COUNT;
    }
    // dump main registers
    for (ARG i = 0; i < dump_regs; i++) {
        dump_rg(emu_st, i);
    }
    if (!full) { // dump special registers even in a small dump
        dump_rg(emu_st, REG_RAD);
        dump_rg(emu_st, REG_RAT);
        dump_rg(emu_st, REG_RSP);
    }
}

/* #endregion */

/* #region Interrupt Handling */

/**
 * Handle interrupts in emulator
 */
void emu_interrupt(EmulatorState *emu_st, UWORD interrupt) {
    printf("--INT: $%08x-- \n", interrupt);
    switch (interrupt) {
    case INTERRUPT_PAUSE: {
        printf("PAUSE");
        size_t pause_bufsize = 256;
        char pause_buf[pause_bufsize];
        util_getln(pause_buf, pause_bufsize);
        break;
    }
    case INTERRUPT_DUMPCPU: {
        emu_dump(emu_st, true);
        break;
    }
    case INTERRUPT_DUMPMEM: {
        // TODO: dump current memory page
        break;
    }
    case INTERRUPT_DUMPSTK: {
        // dump stack
        printf("-- STACK --\n");
        UWORD sp = emu_st->reg[REG_RSP];
        for (UWORD addr = sp; addr < emu_st->mem_sz; addr += sizeof(UWORD)) {
            UWORD data = emu_st->mem[addr + 0] << 0 | emu_st->mem[addr + 1] << 8 | emu_st->mem[addr + 2] << 16 |
                         emu_st->mem[addr + 3] << 24;
            printf(" %0x", data);
        }
        break;
    }
    default: {
        printf("UNKNOWN");
        break;
    }
    }
    printf("\n");
}

/* #endregion */

/* #region Instruction Execution */

/**
 * Execute an instruction in the emulator
 */
void emu_exec(EmulatorState *emu_st, Statement in) {
    switch (in.opcode) {
    case OP_NOP: {
        // do nothing
        break;
    }
    case OP_ADD: {
        emu_st->reg[in.a1] = emu_st->reg[in.a2] + emu_st->reg[in.a3];
        break;
    }
    case OP_SUB: {
        emu_st->reg[in.a1] = emu_st->reg[in.a2] - emu_st->reg[in.a3];
        break;
    }
    case OP_AND: {
        emu_st->reg[in.a1] = emu_st->reg[in.a2] & emu_st->reg[in.a3];
        break;
    }
    case OP_ORR: {
        emu_st->reg[in.a1] = emu_st->reg[in.a2] | emu_st->reg[in.a3];
        break;
    }
    case OP_XOR: {
        emu_st->reg[in.a1] = emu_st->reg[in.a2] ^ emu_st->reg[in.a3];
        break;
    }
    case OP_NOT: {
        emu_st->reg[in.a1] = ~emu_st->reg[in.a2];
        break;
    }
    case OP_LSH: {
        WORD shift = emu_st->reg[in.a3];
        if (shift >= 0) {
            emu_st->reg[in.a1] = emu_st->reg[in.a2] << shift;
        } else {
            emu_st->reg[in.a1] = emu_st->reg[in.a2] >> -shift;
        }
        break;
    }
    case OP_ASH: {
        WORD shift = emu_st->reg[in.a3];
        if (shift >= 0) {
            emu_st->reg[in.a1] = ((WORD)emu_st->reg[in.a2]) << shift;
        } else {
            emu_st->reg[in.a1] = ((WORD)emu_st->reg[in.a2]) >> -shift;
        }
        break;
    }
    case OP_TCU: {
        WORD sign = 0;
        if (emu_st->reg[in.a2] > emu_st->reg[in.a3]) {
            sign = 1;
        } else if (emu_st->reg[in.a2] < emu_st->reg[in.a3]) {
            sign = -1;
        }
        emu_st->reg[in.a1] = sign;
        break;
    }
    case OP_TCS: {
        WORD sign = 0;
        if (((WORD)emu_st->reg[in.a2]) > ((WORD)emu_st->reg[in.a3])) {
            sign = 1;
        } else if (((WORD)emu_st->reg[in.a2]) < ((WORD)emu_st->reg[in.a3])) {
            sign = -1;
        }
        emu_st->reg[in.a1] = sign;
        break;
    }
    case OP_SET: {
        emu_st->reg[in.a1] = in.a2 | (in.a3 << 8);
        break;
    }
    case OP_MOV: {
        emu_st->reg[in.a1] = emu_st->reg[in.a2];
        break;
    }
    case OP_LDW: {
        UWORD addr = emu_st->reg[in.a2];
        emu_st->reg[in.a1] = emu_st->mem[addr + 0] << 0 | emu_st->mem[addr + 1] << 8 | emu_st->mem[addr + 2] << 16 |
                             emu_st->mem[addr + 3] << 24;
        break;
    }
    case OP_STW: {
        UWORD addr = emu_st->reg[in.a1];
        emu_st->mem[addr + 0] = (emu_st->reg[in.a2] >> 0) & 0xff;
        emu_st->mem[addr + 1] = (emu_st->reg[in.a2] >> 8) & 0xff;
        emu_st->mem[addr + 2] = (emu_st->reg[in.a2] >> 16) & 0xff;
        emu_st->mem[addr + 3] = (emu_st->reg[in.a2] >> 24) & 0xff;
        break;
    }
    case OP_INT: {
        UWORD interrupt = emu_st->reg[in.a1];
        emu_interrupt(emu_st, interrupt);
        break;
    }
    case OP_HLT: {
        emu_st->executing = false;
        break;
    }
    case OP_BRX: {
        if (emu_st->reg[in.a1] > 0) {
            emu_st->reg[REG_RPC] = emu_st->reg[in.a2];
        }
        break;
    }
    default:
        break;
    }
}

/* #endregion */

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

        if (emu_st->debug) {
            dump_statement(stmt); // dump statement
        }
        emu_exec(emu_st, stmt); // execute statement
        if (emu_st->debug) {
            emu_dump(emu_st, false); // dump abbrev. state
        }

        emu_st->ticks++;
        if (emu_st->onestep) {
            // TODO: execute commands
            size_t cmd_bufsize = 256;
            char cmd_buf[cmd_bufsize];
            util_getln(cmd_buf, cmd_bufsize);
        }
    }
}

/*
disasm.h
provides emulation capability
*/

#pragma once
#include "disasm.h"
#include "instr.h"
#include <stdbool.h>

const size_t MEMORY_SIZE = 64 * 1024; // 65K

typedef struct {
    ARG *reg;
    BYTE *mem;
    size_t mem_sz;
} EmulatorState;

EmulatorState *emu_init() {
    EmulatorState *st = malloc(sizeof(EmulatorState));
    st->mem_sz = MEMORY_SIZE;
    st->mem = malloc(MEMORY_SIZE * sizeof(BYTE));

    return st;
}

/**
 * Load the program data into memory
 */
void emu_load(EmulatorState *st, int offset, char *program, size_t program_sz) {
    memcpy(st->mem + offset, program, program_sz);
}

void emu_run(EmulatorState *st, ARG entry) {
    // set PC register to entrypoint
    st->reg[0] = entry;
    // start decode loop
    bool executing = true;
    while (executing) {
        // decode instruction
        ARG op = st->mem[st->reg[0]];
        ARG d1 = st->mem[st->reg[0] + 1];
        ARG d2 = st->mem[st->reg[0] + 2];
        ARG d3 = st->mem[st->reg[0] + 3];
        st->reg[0] += 4; // advance PC
        Statement st = {.opcode = op, .a1 = d1, .a2 = d2, .a3 = d3};
        dump_statement(st);
    }
}

void emu_free(EmulatorState *st) {
    // free data
    free(st->reg);
    free(st->mem);
    // free emu state
    free(st);
}

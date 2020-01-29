/*
asm_ext.h
provides extensions to assembler
*/

#pragma once

#include "asm.h"

typedef struct {
    int pos = 0;
    Program *src;
} PseudoAssemblerState;

Statement take_statement(PseudoAssemblerState *pas) { return pas->src->statements[pas->pos++]; }

Program compile_pseudo(Program inp) {
    int statement_buf_size = 128;
    Statement *new_statements = malloc(statement_buf_size * sizeof(new_statements));
    int new_statement_count = 0;
    Program prg = {.statements = new_statements, .status = 0};
    PseudoAssemblerState pas = {.pos = 0, .src = &inp};

    while (pas.pos < inp.statement_count) {
        if (new_statement_count > statement_buf_size - 1) {
            // out of space
            printf("ERROR: out of statement space\n");
            return prg;
        }
        Statement in_stmt = take_statement(&pas);

        // process the statement
        switch (in_stmt.opcode) {
        case OP_JMP: {
            // compile to setting pc
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RPC, .a2 = in_stmt.a1, .a3 = 0};
            new_statements[new_statement_count++] = cmp1;
            break;
        }

        default:
            // copy instruction
            new_statements[new_statement_count++] = in_stmt; // advance counter
            break;
        }
    }

    prg.statement_count = new_statement_count;
    return prg;
}

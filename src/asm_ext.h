/*
asm_ext.h
provides extensions to assembler
*/

#pragma once

#include "asm.h"

typedef struct {
    int pos;
    Program *src;
} PseudoAssemblerState;

Statement take_statement(PseudoAssemblerState *pas) { return pas->src->statements[pas->pos++]; }

Program compile_pseudo(Program inp) {
    int statement_buf_size = 128;
    Statement *new_statements = malloc(statement_buf_size * sizeof(new_statements));
    int new_statement_count = 0;

    Program prg;
    program_init(&prg);
    prg.statements = new_statements;
    prg.entry = inp.entry;
    PseudoAssemblerState pas = {.pos = 0, .src = &inp};

    // TODO: handle offset padding for larger compiles

    while (pas.pos < inp.statement_count) {
        if (new_statement_count > statement_buf_size - 1) {
            // out of space
            printf("ERROR: out of statement space\n");
            return prg;
        }
        Statement in = take_statement(&pas);

        // process the statement
        switch (in.opcode) {
        case OP_JMP: {
            // jmp rA
            // compile to mov reg to pc
            /*
                mov pc rA
            */
            Statement cmp1 = {.opcode = OP_MOV, .a1 = REG_RPC, .a2 = in.a1, .a3 = 0};
            populate_statement(&cmp1);
            new_statements[new_statement_count++] = cmp1;
            break;
        }
        case OP_JMI: {
            // jmi imm
            // compile to setting pc
            /*
                set pc imm
            */
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RPC, .a2 = in.a2, .a3 = in.a3};
            populate_statement(&cmp1);
            new_statements[new_statement_count++] = cmp1;
            break;
        }
        case OP_SWP: {
            // swp rA rB
            // compile to swap using temp
            /*
                mov at rA
                mov rA rB
                mov rB at
            */
            Statement cmp1 = {.opcode = OP_MOV, .a1 = REG_RAT, .a2 = in.a1, .a3 = 0};
            Statement cmp2 = {.opcode = OP_MOV, .a1 = in.a1, .a2 = in.a2, .a3 = 0};
            Statement cmp3 = {.opcode = OP_MOV, .a1 = in.a2, .a2 = REG_RAT, .a3 = 0};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            populate_statement(&cmp3);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            new_statements[new_statement_count++] = cmp3;
            break;
        }
        case OP_ADI: {
            // adi rA imm
            // compile to expand expression
            /*
                set at imm
                add rA rA at
            */
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RAT, .a2 = in.a1, .a3 = 0};
            Statement cmp2 = {.opcode = OP_ADD, .a1 = in.a1, .a2 = in.a1, .a3 = REG_RAT};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            break;
        }
        case OP_SBI: {
            // sbi rA imm
            // compile to expand expression
            /*
                set at imm
                sub rA rA at
            */
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RAT, .a2 = in.a1, .a3 = 0};
            Statement cmp2 = {.opcode = OP_SUB, .a1 = in.a1, .a2 = in.a1, .a3 = REG_RAT};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            break;
        }
        case OP_PSH: {
            // psh rA
            // compile to lower sp and then save
            /*
                set at 4
                sub sp sp at
                stw sp rA
            */
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RAT, .a2 = sizeof(UWORD), .a3 = 0};
            Statement cmp2 = {.opcode = OP_SUB, .a1 = REG_RSP, .a2 = REG_RSP, .a3 = REG_RAT};
            Statement cmp3 = {.opcode = OP_STW, .a1 = REG_RSP, .a2 = in.a1, .a3 = 0};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            populate_statement(&cmp3);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            new_statements[new_statement_count++] = cmp3;
            break;
        }
        case OP_POP: {
            // pop rA
            // compile to load value and then raise sp
            /*
                set at 4
                ldw rA sp
                add sp sp at
            */
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RAT, .a2 = sizeof(UWORD), .a3 = 0};
            Statement cmp2 = {.opcode = OP_LDW, .a1 = in.a1, .a2 = REG_RSP, .a3 = REG_RSP};
            Statement cmp3 = {.opcode = OP_ADD, .a1 = REG_RSP, .a2 = REG_RSP, .a3 = REG_RAT};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            populate_statement(&cmp3);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            new_statements[new_statement_count++] = cmp3;
            break;
        }
        case OP_CAL: {
            // cal rA
            // compile to push pc then jmp to addr
            /*
                set at 16 ; ret addr offset
                add ad at pc ; calculate [pc + offset]
                psh ad
                jmp rA
            */
            Statement cmp1 = {.opcode = OP_SET, .a1 = REG_RAT, .a2 = sizeof(UWORD) * 4, .a3 = 0};
            Statement cmp2 = {.opcode = OP_ADD, .a1 = REG_RAD, .a2 = REG_RAT, .a3 = REG_RPC};
            Statement cmp3 = {.opcode = OP_PSH, .a1 = REG_RAD, .a2 = 0, .a3 = 0};
            Statement cmp4 = {.opcode = OP_JMP, .a1 = in.a1, .a2 = 0, .a3 = 0};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            populate_statement(&cmp3);
            populate_statement(&cmp4);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            new_statements[new_statement_count++] = cmp3;
            new_statements[new_statement_count++] = cmp4;
            break;
        }
        case OP_RET: {
            // ret
            // compile to pop pc to ad then mov
            /*
                pop ad
                jmp ad
            */
            Statement cmp1 = {.opcode = OP_POP, .a1 = REG_RAD, .a2 = 0, .a3 = 0};
            Statement cmp2 = {.opcode = OP_JMP, .a1 = REG_RAD, .a2 = 0, .a3 = 0};
            populate_statement(&cmp1);
            populate_statement(&cmp2);
            new_statements[new_statement_count++] = cmp1;
            new_statements[new_statement_count++] = cmp2;
            break;
            break;
        }
        default:
            // copy instruction
            new_statements[new_statement_count++] = in; // advance counter
            break;
        }
    }

    prg.statement_count = new_statement_count;
    return prg;
}

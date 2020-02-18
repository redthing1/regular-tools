/*
asm_ext.h
provides extensions to assembler
*/

#pragma once

#include "asm.h"

typedef struct {
    int pos;
    SourceProgram *src;
} PseudoAssemblerState;

AStatement take_statement(PseudoAssemblerState *pas) { return buf_get_AStatement(&pas->src->statements, pas->pos++); }

SourceProgram compile_pseudo_instructions(SourceProgram src) {
    SourceProgram prg;
    source_program_init(&prg);
    // copy from source program
    prg.entry = src.entry;
    prg.data = src.data;
    prg.data_size = src.data_size;

    PseudoAssemblerState pas = {.pos = 0, .src = &src};

    // TODO: handle offset padding for larger compiles

    while (pas.pos < src.statements.ct) {
        AStatement in = take_statement(&pas);

        // process the statement
        switch (in.op) {
        case OP_JMP: {
            // jmp rA
            // compile to mov reg to pc
            /*
                mov pc rA
            */
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_MOV, REG_RPC, in.a1.val, 0));
            break;
        }
        case OP_JMI: {
            // jmi imm
            // compile to setting pc
            /*
                set pc imm
            */
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SET, REG_RPC, in.a2.val, in.a3.val));
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
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_MOV, REG_RAT, in.a1.val, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_MOV, in.a1.val, in.a2.val, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_MOV, in.a2.val, REG_RAT, 0));
            break;
        }
        case OP_ADI: {
            // adi rA imm
            // compile to expand expression
            /*
                set at imm
                add rA rA at
            */
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SET, REG_RAT, in.a2.val, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_ADD, in.a1.val, in.a1.val, REG_RAT));
            break;
        }
        case OP_SBI: {
            // sbi rA imm
            // compile to expand expression
            /*
                set at imm
                sub rA rA at
            */
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SET, REG_RAT, in.a2.val, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SUB, in.a1.val, in.a1.val, REG_RAT));
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
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SET, REG_RAT, sizeof(UWORD), 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SUB, REG_RSP, REG_RSP, REG_RAT));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_STW, REG_RSP, in.a1.val, 0));
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
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SET, REG_RAT, sizeof(UWORD), 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_LDW, in.a1.val, REG_RSP, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_ADD, REG_RSP, REG_RSP, REG_RAT));
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
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_SET, REG_RAT, sizeof(UWORD) * 4, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_ADD, REG_RAD, REG_RAT, REG_RPC));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_PSH, REG_RAD, 0, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_JMP, in.a1.val, 0, 0));
            break;
        }
        case OP_RET: {
            // ret
            // compile to pop pc to ad then mov
            /*
                pop ad
                jmp ad
            */
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_POP, REG_RAD, 0, 0));
            buf_push_AStatement(&prg.statements, IMM_STATEMENT(OP_JMP, REG_RAD, 0, 0));
            break;
            break;
        }
        default:
            // copy instruction
            buf_push_AStatement(&prg.statements, in);
            break;
        }
    }

    return prg;
}

SourceProgram simplify_pseudo_2pass(SourceProgram src) {
    // two-pass compile pseudo
    SourceProgram s1 = compile_pseudo_instructions(src);
    free_source_program(s1, false);
    SourceProgram s2 = compile_pseudo_instructions(s1);
    return s2;
}

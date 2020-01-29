/*
instr.h
provides instruction declarations
*/

#pragma once
#include "util.h"
#include <stdint.h>

#define INSTR_SIZE 4

typedef uint8_t BYTE;
typedef BYTE ARG;
typedef ARG OPCODE;

typedef enum {
    INSTR_INV = 0, // invalid instruction
    INSTR_OP = 1 << 0,
    INSTR_K_0_0_0 = 1 << 1,
    INSTR_K_I1 = 1 << 2,
    INSTR_K_R1 = 1 << 3,
    INSTR_K_I2 = 1 << 4,
    INSTR_K_R2 = 1 << 5,
    INSTR_K_I3 = 1 << 6,
    INSTR_K_R3 = 1 << 7,
    INSTR_OP_I = INSTR_OP | INSTR_K_I1,
    INSTR_OP_R = INSTR_OP | INSTR_K_R1,
    INSTR_OP_R_I = INSTR_OP | INSTR_K_R1 | INSTR_K_I2,
    INSTR_OP_R_R = INSTR_OP | INSTR_K_R1 | INSTR_K_R2,
    INSTR_OP_R_R_I = INSTR_OP | INSTR_K_R1 | INSTR_K_R2 | INSTR_K_I3,
    INSTR_OP_R_R_R = INSTR_OP | INSTR_K_R1 | INSTR_K_R2 | INSTR_K_R3,
} InstructionType;

typedef struct {
    OPCODE opcode;
    ARG a1, a2, a3;
    InstructionType type;
    int sz;
} Statement;

typedef struct {
    InstructionType type;
    OPCODE opcode;
    int fin_sz;
} InstructionInfo;

/* #region OPCODE and REG definitions */

// opcodes - BASE
#define OP_NOP 0x00
#define OP_ADD 0x01
#define OP_SUB 0x02
#define OP_AND 0x03
#define OP_ORR 0x04
#define OP_XOR 0x05
#define OP_NOT 0x06
#define OP_LSH 0x07
#define OP_ASH 0x08
#define OP_TCU 0x09
#define OP_TCS 0x0a
#define OP_SET 0x0b
#define OP_MOV 0x0c
#define OP_LDW 0x0d
#define OP_STW 0x0e
#define OP_LDB 0x0f
#define OP_STB 0x10

// opcodes - _ad/hardware
#define OP_HLT 0x70

// opcodes - _ad/pseudo
#define OP_JMP 0xa0
#define OP_JMI 0xa1
#define OP_SWP 0xb0

// registers
#define REG_RXX 0x70
#define REG_RPC 0x00
#define REG_R01 0x01
#define REG_R02 0x02
#define REG_R03 0x03
#define REG_R04 0x04
#define REG_R05 0x05
#define REG_R06 0x06
#define REG_R07 0x07
#define REG_R08 0x08
#define REG_R09 0x09
#define REG_R10 0x0a
#define REG_R11 0x0b
#define REG_R12 0x0c
#define REG_R13 0x0d
#define REG_R14 0x0e
#define REG_R15 0x0f
#define REG_R16 0x10
#define REG_R17 0x11
#define REG_R18 0x12
#define REG_R19 0x13
#define REG_R20 0x14
#define REG_R21 0x15
#define REG_R22 0x16
#define REG_R23 0x17
#define REG_R24 0x18
#define REG_R25 0x19
#define REG_R26 0x1a
#define REG_R27 0x1b
#define REG_R28 0x1c
#define REG_R29 0x1d
#define REG_RSP 0x1e
#define REG_RAT 0x1f

/* #endregion */

InstructionInfo get_instruction_info(char *mnem) {
    if (streq(mnem, "nop")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP, .opcode = OP_NOP};
    } else if (streq(mnem, "add")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_ADD};
    } else if (streq(mnem, "sub")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_SUB};
    } else if (streq(mnem, "and")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_AND};
    } else if (streq(mnem, "orr")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_ORR};
    } else if (streq(mnem, "xor")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_XOR};
    } else if (streq(mnem, "not")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R, .opcode = OP_NOT};
    } else if (streq(mnem, "lsh")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_LSH};
    } else if (streq(mnem, "ash")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_ASH};
    } else if (streq(mnem, "tcu")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_TCU};
    } else if (streq(mnem, "tcs")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R_R, .opcode = OP_TCS};
    } else if (streq(mnem, "set")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_I, .opcode = OP_SET};
    } else if (streq(mnem, "mov")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R, .opcode = OP_MOV};
    } else if (streq(mnem, "ldw")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R, .opcode = OP_LDW};
    } else if (streq(mnem, "stw")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R, .opcode = OP_STW};
    } else if (streq(mnem, "ldb")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R, .opcode = OP_LDB};
    } else if (streq(mnem, "stb")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R_R, .opcode = OP_STB};
    } else if (streq(mnem, "hlt")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP, .opcode = OP_HLT};
    } else if (streq(mnem, "jmp")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_R, .opcode = OP_JMP};
    } else if (streq(mnem, "jmi")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE, .type = INSTR_OP_I, .opcode = OP_JMI};
    } else if (streq(mnem, "swp")) {
        return (InstructionInfo){.fin_sz = INSTR_SIZE * 3, .type = INSTR_OP_R_R, .opcode = OP_SWP};
    } else {
        // unrecognized mnem
        return (InstructionInfo){.type = INSTR_INV, .opcode = OP_NOP};
    }
}

const char *get_instruction_mnem(OPCODE op) {
    switch (op) {
    case OP_NOP:
        return "nop";
    case OP_ADD:
        return "add";
    case OP_SUB:
        return "sub";
    case OP_AND:
        return "and";
    case OP_ORR:
        return "orr";
    case OP_XOR:
        return "xor";
    case OP_NOT:
        return "not";
    case OP_LSH:
        return "lsh";
    case OP_ASH:
        return "ash";
    case OP_TCU:
        return "tcu";
    case OP_TCS:
        return "tcs";
    case OP_SET:
        return "set";
    case OP_MOV:
        return "mov";
    case OP_LDW:
        return "ldw";
    case OP_STW:
        return "stw";
    case OP_LDB:
        return "ldb";
    case OP_STB:
        return "stb";
    case OP_HLT:
        return "hlt";
    default:
        return NULL; // unrecognized mnemonic
        break;
    }
}

ARG get_register(char *mnem) {
    if (streq(mnem, "rX")) {
        return REG_RXX;
    } else if (streq(mnem, "r1")) {
        return REG_R01;
    } else if (streq(mnem, "r2")) {
        return REG_R02;
    } else if (streq(mnem, "r3")) {
        return REG_R03;
    } else if (streq(mnem, "r4")) {
        return REG_R04;
    } else if (streq(mnem, "r5")) {
        return REG_R05;
    } else if (streq(mnem, "r6")) {
        return REG_R06;
    } else if (streq(mnem, "r7")) {
        return REG_R07;
    } else if (streq(mnem, "r8")) {
        return REG_R08;
    } else if (streq(mnem, "r9")) {
        return REG_R09;
    } else if (streq(mnem, "r10")) {
        return REG_R10;
    } else if (streq(mnem, "r11")) {
        return REG_R11;
    } else if (streq(mnem, "r12")) {
        return REG_R12;
    } else if (streq(mnem, "r13")) {
        return REG_R13;
    } else if (streq(mnem, "r14")) {
        return REG_R14;
    } else if (streq(mnem, "r15")) {
        return REG_R15;
    }
    // unrecognized mnem
    return REG_RXX;
}

const char *get_register_name(ARG reg) {
    switch (reg) {
    case REG_RXX:
        return "rX";
    case REG_RPC:
        return "pc";
    case REG_R01:
        return "r1";
    case REG_R02:
        return "r2";
    case REG_R03:
        return "r3";
    case REG_R04:
        return "r4";
    case REG_R05:
        return "r5";
    case REG_R06:
        return "r6";
    case REG_R07:
        return "r7";
    case REG_R08:
        return "r8";
    case REG_RAT:
        return "at";
    case REG_RSP:
        return "sp";
    default:
        return NULL; // unrecognized mnemonic
    }
}

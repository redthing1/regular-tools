/*
instr.h
provides instruction declarations
*/

#pragma once
#include "util.h"
#include <stdint.h>

typedef uint8_t OPCODE;
typedef uint8_t ARG;

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
} Statement;

typedef struct {
    InstructionType type;
    OPCODE opcode;
} InstructionInfo;

/* #region OPCODE and REG definitions */

// opcodes
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

// registers
#define REG_RXX 0x01
#define REG_RPC 0x70
#define REG_R01 0x71
#define REG_R02 0x72
#define REG_R03 0x73
#define REG_R04 0x74
#define REG_R05 0x75
#define REG_R06 0x76
#define REG_R07 0x77
#define REG_R08 0x78
#define REG_R09 0x79
#define REG_R10 0x7a
#define REG_R11 0x7b
#define REG_R12 0x7c
#define REG_R13 0x7d
#define REG_R14 0x7e
#define REG_R15 0x7f
#define REG_R16 0x80
#define REG_R17 0x81
#define REG_R18 0x82
#define REG_R19 0x83
#define REG_R20 0x84
#define REG_R21 0x85
#define REG_R22 0x86
#define REG_R23 0x87
#define REG_R24 0x88
#define REG_R25 0x89
#define REG_R26 0x8a
#define REG_R27 0x8b
#define REG_R28 0x8c
#define REG_R29 0x8d
#define REG_RSP 0x8e
#define REG_RAT 0x8f

/* #endregion */

InstructionInfo get_instruction_info(char *mnem) {
    if (streq(mnem, "nop")) {
        return (InstructionInfo){.type = INSTR_OP, .opcode = OP_NOP};
    } else if (streq(mnem, "add")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_ADD};
    } else if (streq(mnem, "sub")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_SUB};
    } else if (streq(mnem, "and")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_AND};
    } else if (streq(mnem, "orr")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_ORR};
    } else if (streq(mnem, "xor")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_XOR};
    } else if (streq(mnem, "not")) {
        return (InstructionInfo){.type = INSTR_OP_R_R, .opcode = OP_NOT};
    } else if (streq(mnem, "lsh")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_LSH};
    } else if (streq(mnem, "ash")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_ASH};
    } else if (streq(mnem, "tcu")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_TCU};
    } else if (streq(mnem, "tcs")) {
        return (InstructionInfo){.type = INSTR_OP_R_R_R, .opcode = OP_TCS};
    } else if (streq(mnem, "set")) {
        return (InstructionInfo){.type = INSTR_OP_R_I, .opcode = OP_SET};
    } else if (streq(mnem, "mov")) {
        return (InstructionInfo){.type = INSTR_OP_R_R, .opcode = OP_MOV};
    } else if (streq(mnem, "ldw")) {
        return (InstructionInfo){.type = INSTR_OP_R_R, .opcode = OP_LDW};
    } else if (streq(mnem, "stw")) {
        return (InstructionInfo){.type = INSTR_OP_R_R, .opcode = OP_STW};
    } else if (streq(mnem, "ldb")) {
        return (InstructionInfo){.type = INSTR_OP_R_R, .opcode = OP_LDB};
    } else if (streq(mnem, "stb")) {
        return (InstructionInfo){.type = INSTR_OP_R_R, .opcode = OP_STB};
    } else {
        // unrecognized mnem
        return (InstructionInfo){.type = INSTR_INV, .opcode = OP_NOP};
    }
}

ARG get_register(char *mnem) {
    if (streq(mnem, "rX")) {
        return REG_RXX;
    } else if (streq(mnem, "r1")) {
        return REG_R01;
    }
    // unrecognized mnem
    return REG_RXX;
}

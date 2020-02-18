/*
instr.h
provides instruction declarations
*/

#pragma once
#include "util.h"
#include <stdint.h>

#define INSTR_SIZE 4

typedef uint8_t BYTE;   // byte size
typedef int32_t WORD;   // word size
typedef uint32_t UWORD; // unsigned word

typedef BYTE ARG;   // args are one byte
typedef ARG OPCODE; // opcodes are one arg

#define HEADER_SIZE 8 // header size

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
} Instruction;

typedef struct {
    InstructionType type; // INSTR_OP_I, INSTR_OP_R, etc.
    OPCODE opcode;
    int sz; // final size of instruction
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
#define OP_HLT 0xff
#define OP_INT 0x71
#define OP_BRX 0x72

// opcodes - _ad/pseudo
#define OP_JMP 0xa0
#define OP_JMI 0xa1
#define OP_PSH 0xa2
#define OP_POP 0xa3
#define OP_CAL 0xa4
#define OP_RET 0xa5
#define OP_SWP 0xb0
#define OP_ADI 0xb1
#define OP_SBI 0xb2

// registers
#define REG_RX 0xff
#define REG_RPC 0x00
#define REG_R1 0x01
#define REG_R2 0x02
#define REG_R3 0x03
#define REG_R4 0x04
#define REG_R5 0x05
#define REG_R6 0x06
#define REG_R7 0x07
#define REG_R8 0x08
#define REG_R9 0x09
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
#define REG_RAD 0x1d
#define REG_RAT 0x1e
#define REG_RSP 0x1f

/* #endregion */

#define INSTRINFO(WORDS, TYPE, OPCODE)                                                                                 \
    (InstructionInfo) { .sz = INSTR_SIZE * WORDS, .type = TYPE, .opcode = OPCODE }

InstructionInfo get_instruction_info(char *mnem) {
    if (streq(mnem, "nop")) {
        return INSTRINFO(1, INSTR_OP, OP_NOP);
    } else if (streq(mnem, "add")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_ADD);
    } else if (streq(mnem, "sub")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_SUB);
    } else if (streq(mnem, "and")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_AND);
    } else if (streq(mnem, "orr")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_ORR);
    } else if (streq(mnem, "xor")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_XOR);
    } else if (streq(mnem, "not")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_NOT);
    } else if (streq(mnem, "lsh")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_LSH);
    } else if (streq(mnem, "ash")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_ASH);
    } else if (streq(mnem, "tcu")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_TCU);
    } else if (streq(mnem, "tcs")) {
        return INSTRINFO(1, INSTR_OP_R_R_R, OP_TCS);
    } else if (streq(mnem, "set")) {
        return INSTRINFO(1, INSTR_OP_R_I, OP_SET);
    } else if (streq(mnem, "mov")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_MOV);
    } else if (streq(mnem, "ldw")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_LDW);
    } else if (streq(mnem, "stw")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_STW);
    } else if (streq(mnem, "ldb")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_LDB);
    } else if (streq(mnem, "stb")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_STB);
    } else if (streq(mnem, "hlt")) {
        return INSTRINFO(1, INSTR_OP, OP_HLT);
    } else if (streq(mnem, "int")) {
        return INSTRINFO(1, INSTR_OP_R, OP_INT);
    } else if (streq(mnem, "brx")) {
        return INSTRINFO(1, INSTR_OP_R_R, OP_BRX);
    } else if (streq(mnem, "jmp")) {
        return INSTRINFO(1, INSTR_OP_R, OP_JMP);
    } else if (streq(mnem, "jmi")) {
        return INSTRINFO(1, INSTR_OP_I, OP_JMI);
    } else if (streq(mnem, "psh")) {
        return INSTRINFO(3, INSTR_OP_R, OP_PSH);
    } else if (streq(mnem, "pop")) {
        return INSTRINFO(3, INSTR_OP_R, OP_POP);
    } else if (streq(mnem, "cal")) {
        return INSTRINFO(6, INSTR_OP_R, OP_CAL);
    } else if (streq(mnem, "ret")) {
        return INSTRINFO(4, INSTR_OP, OP_RET);
    } else if (streq(mnem, "swp")) {
        return INSTRINFO(3, INSTR_OP_R_R, OP_SWP);
    } else if (streq(mnem, "adi")) {
        return INSTRINFO(2, INSTR_OP_R_I, OP_ADI);
    } else if (streq(mnem, "sbi")) {
        return INSTRINFO(2, INSTR_OP_R_I, OP_SBI);
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
    case OP_INT:
        return "int";
    case OP_BRX:
        return "brx";
    case OP_JMP:
        return "jmp";
    case OP_JMI:
        return "jmi";
    case OP_PSH:
        return "psh";
    case OP_POP:
        return "pop";
    case OP_CAL:
        return "cal";
    case OP_RET:
        return "ret";
    case OP_SWP:
        return "swp";
    case OP_ADI:
        return "adi";
    case OP_SBI:
        return "sbi";
    default:
        return NULL; // unrecognized mnemonic
        break;
    }
}

#define REG(num) REG_R##num
#define REG_STREQ(num)                                                                                                 \
    else if (streq(mnem, "r" #num)) {                                                                                  \
        return REG(num);                                                                                               \
    }

ARG get_register(char *mnem) {
    if (streq(mnem, "pc")) {
        return REG_RPC;
    }
    REG_STREQ(1)
    REG_STREQ(2)
    REG_STREQ(3)
    REG_STREQ(4)
    REG_STREQ(5)
    REG_STREQ(6)
    REG_STREQ(7)
    REG_STREQ(8)
    REG_STREQ(9)
    REG_STREQ(10)
    REG_STREQ(11)
    REG_STREQ(12)
    REG_STREQ(13)
    REG_STREQ(14)
    REG_STREQ(15)
    REG_STREQ(16)
    REG_STREQ(17)
    REG_STREQ(18)
    REG_STREQ(19)
    REG_STREQ(20)
    REG_STREQ(21)
    REG_STREQ(22)
    REG_STREQ(24)
    REG_STREQ(25)
    REG_STREQ(26)
    REG_STREQ(27)
    REG_STREQ(28)
    else if (streq(mnem, "r29") || streq(mnem, "ad")) {
        return REG_RAT;
    }
    else if (streq(mnem, "at")) {
        return REG_RAT;
    }
    else if (streq(mnem, "sp")) {
        return REG_RSP;
    }
    // unrecognized mnem
    return REG_RX;
}

#define REG_NAMECASE(num)                                                                                              \
    case REG(num):                                                                                                     \
        return "r" #num;

const char *get_register_name(ARG reg) {
    switch (reg) {
        REG_NAMECASE(X);
    case REG_RPC:
        return "pc";
        REG_NAMECASE(1);
        REG_NAMECASE(2);
        REG_NAMECASE(3);
        REG_NAMECASE(4);
        REG_NAMECASE(5);
        REG_NAMECASE(6);
        REG_NAMECASE(7);
        REG_NAMECASE(8);
        REG_NAMECASE(9);
        REG_NAMECASE(10);
        REG_NAMECASE(11);
        REG_NAMECASE(12);
        REG_NAMECASE(13);
        REG_NAMECASE(14);
        REG_NAMECASE(15);
        REG_NAMECASE(16);
        REG_NAMECASE(17);
        REG_NAMECASE(18);
        REG_NAMECASE(19);
        REG_NAMECASE(20);
        REG_NAMECASE(21);
        REG_NAMECASE(22);
        REG_NAMECASE(23);
        REG_NAMECASE(24);
        REG_NAMECASE(25);
        REG_NAMECASE(26);
        REG_NAMECASE(27);
        REG_NAMECASE(28);
    case REG_RAD:
        return "ad";
    case REG_RAT:
        return "at";
    case REG_RSP:
        return "sp";
    default:
        return NULL; // unrecognized mnemonic
    }
}

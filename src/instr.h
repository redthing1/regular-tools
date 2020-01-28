/*
instr.h
provides instruction declarations
*/

#pragma once

typedef char OPCODE;
typedef char ARG;

typedef struct {
    OPCODE opcode;
    ARG a1, a2, a3;
} Statement;

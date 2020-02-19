/*
asm.h
provides lexer and parser for the assembler
*/

#pragma once

#include "instr.h"
#include "lex.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #region Parser */

typedef struct {
    char *label; // label name
    int offset;  // offset to label
} RefValueSource;

typedef enum { VS_IMM, VS_REF } ValueSourceKind;

typedef struct {
    ValueSourceKind kind; // IMM, REF
    uint8_t val;          // immediate value
    RefValueSource ref;   // reference to another
} ValueSource;

typedef struct {
    OPCODE op;
    ValueSource a1, a2, a3;
} AStatement;

#define IMM_STATEMENT(OPCODE, A1, A2, A3)                                                                              \
    (AStatement) {                                                                                                     \
        .op = OPCODE, .a1 = (ValueSource){.kind = VS_IMM, .val = A1}, .a2 = (ValueSource){.kind = VS_IMM, .val = A2},  \
        .a3 = (ValueSource) {                                                                                          \
            .kind = VS_IMM, .val = A3                                                                                  \
        }                                                                                                              \
    }

BUFFIE_OF(AStatement)

typedef struct {
    Buffie_AStatement statements;
    uint16_t entry;
    BYTE *data;
    uint16_t data_size;
    int status;
} SourceProgram;

typedef struct {
    Instruction *instructions;
    uint16_t instruction_count;
    BYTE *data;
    uint16_t data_size;
} CompiledProgram;

typedef struct {
    char *name;
    int offset;
} LabelDef;

BUFFIE_OF(LabelDef)

typedef struct {
    LexResult *lexed;
    int token;              // token index
    int cpos;               // char position of reading
    int offset;             // binary output position
    Buffie_LabelDef labels; // label buffer
} ParserState;

void source_program_init(SourceProgram *p) {
    buf_alloc_AStatement(&p->statements, 128);
    p->status = 0;
    p->data = NULL;
    p->data_size = 0;
}

void parser_state_cleanup(ParserState *st) {
    // clean up labels
    for (size_t i = 0; i < st->labels.ct; i++) {
        LabelDef ld = buf_get_LabelDef(&st->labels, i);
        free(ld.name);
    }
}

Token peek_token(ParserState *st) {
    if (st->token > st->lexed->token_count - 1) {
        return (Token){.kind = UNKNOWN};
    }
    return buf_get_Token(&st->lexed->tokens, st->token);
}

Token take_token(ParserState *st) {
    Token tok = peek_token(st);
    st->token++;
    st->cpos += strlen(tok.cont);
    return tok;
}

Token expect_token(ParserState *st, CharType type) {
    Token next = peek_token(st);
    CharType next_type = next.kind;
    if ((next_type & type) > 0) {
        // expected token found
        return take_token(st);
    } else {
        // expected token not found
        printf("unexpected token#%d @%d: %s [%d]\n", st->token, st->cpos, next.cont, next.kind);
        return (Token){.cont = NULL, .kind = UNKNOWN};
    }
}

uint32_t parse_numeric(ParserState *st) {
    // interpret numeric token
    Token num_tok = expect_token(st, NUMERIC_CONSTANT);
    char pfx = num_tok.cont[0];
    // create a new string without the prefix
    int num_len = strlen(num_tok.cont) - 1;
    char *num_str = malloc(num_len + 1);
    strncpy(num_str, num_tok.cont + 1, num_len);
    num_str[num_len] = '\0';
    // convert base
    uint32_t val = 0;
    switch (pfx) {
    case '$': {
        // interpret as base-16
        val = (int)strtol(num_str, NULL, 16);
        break;
    }
    case '.': {
        // interpret as base-10
        val = atoi(num_str);
        break;
    }
    default:
        // invalid numeric
        printf("ERR: invalid numeric prefix %c", pfx);
    }
    free(num_str); // free numstr
    return val;
}

void define_macro(ParserState *st, const char *name) {}

void define_label(ParserState *st, const char *name) {
    char *label_name = malloc(strlen(name) + 1);
    strcpy(label_name, name);
    LabelDef ld = {.name = label_name, .offset = st->offset};
    buf_push_LabelDef(&st->labels, ld);
}

int resolve_label(ParserState *st, char *name) {
    // find the defined label
    for (size_t i = 0; i < st->labels.ct; i++) {
        LabelDef lb = buf_get_LabelDef(&st->labels, i);
        if (streq(lb.name, name)) {
            return lb.offset;
        }
    }
    printf("ERROR: failed to resolve label %s\n", name);
    return 0; // not resolved
}

SourceProgram parse(LexResult lexed) {
    ParserState st = {.lexed = &lexed, .token = 0, .cpos = 0, .offset = 0};
    buf_alloc_LabelDef(&st.labels, 16);

    SourceProgram src;
    source_program_init(&src);

    // entry label
    char *entry_label = NULL;

    // emit the entry jump (as nop)
    buf_push_AStatement(&src.statements, IMM_STATEMENT(OP_NOP, 0, 0, 0));

    // parse the lex result into a list of instructions
    while (st.token < lexed.token_count) {
        Token next = peek_token(&st);
        switch (next.kind) {
        case DIRECTIVE: { // handle directive
            Token dir = take_token(&st);
            if (streq(dir.cont, "#entry")) { // entrypoint directive
                // following label has the entry point
                expect_token(&st, MARK);
                Token label_ref = expect_token(&st, IDENTIFIER);
                entry_label = label_ref.cont;   // store entry label
            } else if (streq(dir.cont, "#d")) { // data directive
                expect_token(&st, PACK_START);  // eat pack start
                // check pack type indicator
                Token pack_type_indicator = expect_token(&st, ALPHA | QUOT);
                size_t pack_len = 0; // size of packed data

                switch (pack_type_indicator.kind) {
                case ALPHA: { // byte pack
                    Token pack = expect_token(&st, NUMERIC_CONSTANT);
                    pack_len = strlen(pack.cont);
                    if (pack_len % 2 != 0) {
                        // odd number of half-bytes, invalid
                        printf("ERROR: invalid data (must be even)");
                    }
                    pack_len = pack_len / 2;              // divide by two because 0xff = 1 byte
                    BYTE *pack_data = datahex(pack.cont); // convert data from hex
                    // write the pack data to the binary
                    if (!src.data) {
                        src.data = malloc(sizeof(BYTE) * pack_len);
                    } else {
                        src.data = realloc(src.data, sizeof(BYTE) * (src.data_size + pack_len));
                    }
                    // copy the data
                    memcpy(src.data + src.data_size, pack_data, pack_len);
                    free(pack_data); // free decoded data
                    break;
                }
                case QUOT: {
                    Token pack = take_token(&st); // any following token is valid
                    pack_len = strlen(pack.cont);
                    // copy string from token to data
                    memcpy(src.data + src.data_size, pack.cont, pack_len);
                    break;
                }
                default:
                    printf("unrecognized pack type %s\n", pack_type_indicator.cont);
                    break;
                }

                // update offset
                src.data_size += pack_len;
                st.offset += pack_len;
                printf("data block, len: $%04x\n", (UWORD)pack_len);
            }
            break;
        }
        case IDENTIFIER: {
            Token iden = expect_token(&st, IDENTIFIER);
            Token next = peek_token(&st);
            switch (next.kind) {
            case MARK: {                      // label def
                expect_token(&st, MARK);      // eat the mark
                define_label(&st, iden.cont); // create label
                break;
            }
            case BIND: {                      // macro def
                expect_token(&st, BIND);      // eat the bind
                define_macro(&st, iden.cont); // define the macro
                break;
            }
            default: { // instruction
                break;
            }
            }
            // TODO: handle id
            take_token(&st); // eat the tok
            break;
        }
        default:
            printf("ERR: unexpected token #%d\n", st.token);
            src.status = 1;
            parser_state_cleanup(&st);
            return src;
        }
    }

    // check for entry point label
    if (entry_label) {
        // resolve the label and replace the entry jump
        UWORD entry_addr = resolve_label(&st, entry_label);
        buf_set_AStatement(&src.statements, 0, IMM_STATEMENT(OP_JMI, entry_addr, 0, 0));
    }

    parser_state_cleanup(&st);

    // update program information
    return src;
}

void compiled_program_init(CompiledProgram *cmp) {
    cmp->instructions = NULL;
    cmp->instruction_count = 0;
    cmp->data = NULL;
    cmp->data_size = 0;
}

Instruction compile_statement(const AStatement *st) {
    return (Instruction){.opcode = st->op, .a1 = st->a1.val, st->a2.val, st->a3.val};
}

CompiledProgram compile_program(SourceProgram src) {
    CompiledProgram cmp;
    compiled_program_init(&cmp);

    // this will only work if src is fully simplified (1 Statement : 1 Instruction)
    cmp.instruction_count = src.statements.sz;
    cmp.instructions = malloc(sizeof(Instruction) * cmp.instruction_count);
    // copy data
    cmp.data = src.data;
    cmp.data_size = src.data_size;

    // step-by-step convert the program
    for (size_t i = 0; i < src.statements.sz; i++) {
        AStatement st = buf_get_AStatement(&src.statements, i);
        // we assume that all statement arguments are resolved
        Instruction in = compile_statement(&st);
        cmp.instructions[i] = in; // save the instruction
    }

    return cmp;
}

void free_source_program(SourceProgram src, bool free_data) {
    // free statement buffer
    buf_free_AStatement(&src.statements);
    if (free_data && src.data) {
        // free data
        free(src.data);
    }
}

void free_compiled_program(CompiledProgram cmp) {
    // we don't touch data.
    // free the instructions
    free(cmp.instructions);
    cmp.instructions = NULL;
    cmp.instruction_count = 0;
}

/* #endregion */

/* #region Binary */

void write_short(FILE *ouf, uint8_t v) {
    char w0 = (v >> 0) & 0xff;
    char w1 = (v >> 8) & 0xff;
    fwrite(&w0, sizeof(w0), 1, ouf);
    fwrite(&w1, sizeof(w1), 1, ouf);
}

void write_instruction(FILE *ouf, Instruction *in) {
    char w = '\0';
    // write binary statement data
    w = in->opcode;
    fwrite(&w, sizeof(w), 1, ouf);
    // write binary args
    w = in->a1;
    fwrite(&w, sizeof(w), 1, ouf);
    w = in->a2;
    fwrite(&w, sizeof(w), 1, ouf);
    w = in->a3;
    fwrite(&w, sizeof(w), 1, ouf);
}

void write_compiled_program(FILE *ouf, CompiledProgram cmp) {
    char w = '\0';

    // write data
    if (cmp.data) {
        for (int i = 0; i < cmp.data_size; i++) {
            w = cmp.data[i];
            fwrite(&w, sizeof(w), 1, ouf);
        }
        printf("data[%d] \n", (int)cmp.data_size);
    }

    // write code
    size_t code_offset = 0;

    // write instructions
    for (int i = 0; i < cmp.instruction_count; i++) {
        Instruction in = cmp.instructions[i];
        InstructionInfo info = get_instruction_info_op(in.opcode);
        write_instruction(ouf, &in);
        code_offset += info.sz;
    }
    printf("code[%d] \n", (int)code_offset);
}

/* #endregion */

/* #region Debugging */

void dump_instruction(Instruction in, bool rich) {
    const char *op_name = get_instruction_mnem(in.opcode);
    InstructionInfo info = get_instruction_info(op_name);

    if (rich) {
        printf("[%3s]", op_name);
    } else {
        printf("%3s", op_name);
    }
    if ((info.type & INSTR_K_R1) > 0) {
        printf(" %-3s", get_register_name(in.a1));
    }
    if ((info.type & INSTR_K_R2) > 0) {
        printf(" %-3s", get_register_name(in.a2));
    }
    if ((info.type & INSTR_K_R3) > 0) {
        printf(" %-3s", get_register_name(in.a3));
    }
    if ((info.type & INSTR_K_I1) > 0) {
        uint32_t v = in.a1 | (in.a2 << 8) | (in.a3 << 16);
        printf(" $%04x", v);
    } else if ((info.type & INSTR_K_I2) > 0) {
        uint32_t v = in.a2 | (in.a3 << 8);
        printf(" $%04x", v);
    } else if ((info.type & INSTR_K_I3) > 0) {
        printf(" $%04x", in.a3);
    }
    printf("\n");
}

void dump_statement(AStatement st) {
    // compile the statement
    Instruction in = compile_statement(&st);
    dump_instruction(in, true);
}

void dump_source_program(SourceProgram src) {
    printf("entry:     $%04x\n", src.entry);
    uint16_t code_size = src.statements.ct * INSTR_SIZE;
    printf("code size: $%04x\n", code_size);
    printf("data size: $%04x\n", src.data_size);
    int offset = HEADER_SIZE + src.data_size;
    for (size_t i = 0; i < src.statements.ct; i++) {
        AStatement st = buf_get_AStatement(&src.statements, i);
        InstructionInfo info = get_instruction_info_op(st.op);
        printf("%04x ", offset);
        dump_statement(st);
        offset += info.sz;
    }
}

void dump_compiled_program(CompiledProgram cmp, bool rich) {
    printf("code size: $%04x\n", cmp.instruction_count * INSTR_SIZE);
    printf("data size: $%04x\n", cmp.data_size);
    int offset = HEADER_SIZE + cmp.data_size;
    for (size_t i = 0; i < cmp.instruction_count; i++) {
        Instruction in = cmp.instructions[i];
        if (rich)
            printf("%04x ", offset);
        dump_instruction(in, rich);
        offset += INSTR_SIZE;
    }
}

/* #endregion */

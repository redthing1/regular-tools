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
    char *label;    // label name
    int add_offset; // add offset
} RefValueSource;

typedef enum { VS_IMM, VS_REF } ValueSourceKind;

typedef struct {
    ValueSourceKind kind; // IMM, REF
    uint32_t val;         // immediate value
    RefValueSource ref;   // reference to another
} ValueSource;

typedef struct {
    OPCODE op;
    ValueSource a1, a2, a3;
} AStatement;

#define IMM_ARG(A1)                                                                                                    \
    (ValueSource) { .kind = VS_IMM, .val = A1 }

#define IMM_STATEMENT(OPCODE, A1, A2, A3)                                                                              \
    (AStatement) {                                                                                                     \
        .op = OPCODE, .a1 = IMM_ARG(A1), .a2 = (ValueSource){.kind = VS_IMM, .val = A2}, .a3 = (ValueSource) {         \
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

typedef enum { MACROARG_REG, MACROARG_VAL } MacroArgType;

typedef struct {
    MacroArgType type; // REG, VAL
    char *name;        // argument name
} MacroArg;

BUFFIE_OF(MacroArg)

typedef struct {
    const char *mnem;
    const char *a1, *a2, *a3;
} RawStatement;

BUFFIE_OF(RawStatement)

typedef struct {
    char *name;
    Buffie_MacroArg args;
    Buffie_RawStatement statements;
} MacroDef;

BUFFIE_OF(MacroDef)

typedef struct {
    LexResult *lexed;
    int token;              // token index
    int cpos;               // char position of reading
    int offset;             // binary output position
    Buffie_LabelDef labels; // label buffer
    Buffie_MacroDef macros; // macro buffer
} ParserState;

void source_program_init(SourceProgram *p) {
    buf_alloc_AStatement(&p->statements, 128);
    p->entry = 0;
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
    buf_free_LabelDef(&st->labels);
    // clean up macros
    for (size_t i = 0; i < st->macros.ct; i++) {
        MacroDef md = buf_get_MacroDef(&st->macros, i);
        free(md.name);
        for (size_t j = 0; j < md.args.ct; j++) {
            MacroArg arg = buf_get_MacroArg(&md.args, j);
            free(arg.name);
        }
        buf_free_MacroArg(&md.args);
        buf_free_RawStatement(&md.statements);
    }
    buf_free_MacroDef(&st->macros);
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

ValueSource read_value_arg(ParserState *st) {
    Token next = peek_token(st);
    ValueSource vs;
    vs.kind = VS_IMM;
    vs.val = 0;
    if (next.kind == MARK) {
        expect_token(st, MARK); // eat the mark
        Token label_ref_tok = expect_token(st, IDENTIFIER);
        vs.kind = VS_REF;
        vs.ref = (RefValueSource){.label = label_ref_tok.cont, .add_offset = 0};
        Token pk_offset = peek_token(st);
        if (pk_offset.kind == OFFSET) {
            expect_token(st, OFFSET); // eat the offset token
            uint32_t offset_val = parse_numeric(st);
            vs.ref.add_offset = offset_val;
        }
        return vs;
    } else if (next.kind == NUMERIC_CONSTANT) {
        // interpret numeric token
        Token num_tok = expect_token(st, NUMERIC_CONSTANT);
        char prefix = num_tok.cont[0];
        // create a new string without the prefix
        int num_len = strlen(num_tok.cont) - 1;
        char *num_str = malloc(num_len + 1);
        strncpy(num_str, num_tok.cont + 1, num_len);
        num_str[num_len] = '\0';
        // convert base
        uint32_t val = 0;
        switch (prefix) {
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
            printf("ERR: invalid numeric prefix %c", prefix);
        }
        free(num_str); // free numstr
        vs.kind = VS_IMM;
        vs.val = val;
        return vs;
    } else {
        printf("ERR: unrecognized token %s for value arg\n", next.cont);
    }
    return vs;
}

AStatement read_statement(ParserState *pst, const char *mnem, const char *a1, const char *a2, const char *a3) {
    // given guaranteed validation of opcode
    InstructionInfo info = get_instruction_info(mnem);
    AStatement raw_stmt = IMM_STATEMENT(info.opcode, 0, 0, 0);
    raw_stmt.op = info.opcode;

    // read the instruction data
    if ((info.type & INSTR_K_R1) > 0) {
        raw_stmt.a1 = IMM_ARG(get_register(a1));
    }
    if ((info.type & INSTR_K_R2) > 0) {
        raw_stmt.a2 = IMM_ARG(get_register(a2));
    }
    if ((info.type & INSTR_K_R3) > 0) {
        raw_stmt.a3 = IMM_ARG(get_register(a3));
    }

    if ((info.type & INSTR_K_I1) > 0) {
        raw_stmt.a1 = read_value_arg(pst);
    } else if ((info.type & INSTR_K_I2) > 0) {
        raw_stmt.a2 = read_value_arg(pst);
    } else if ((info.type & INSTR_K_I3) > 0) {
        raw_stmt.a3 = read_value_arg(pst);
    }

    return raw_stmt;
}

int match_macro_argdef(MacroDef *md, const char *arg) {
    for (size_t i = 0; i < md->args.ct; i++) {
        MacroArg arg_def = buf_get_MacroArg(&md->args, i);
        if (streq(arg_def.name, arg)) {
            return i;
        }
    }
    return -1;
}

void expand_macro(ParserState *pst, MacroDef *md, Buffie_AStatement *statements) {
    // printf("unrolling macro: %s\n", md->name);
    const char *inputs[md->args.ct];
    // save the input args
    for (size_t i = 0; i < md->args.ct; i++) {
        Token input_arg = expect_token(pst, IDENTIFIER);
        inputs[i] = input_arg.cont;
    }
    // unroll and expand the macro
    for (size_t i = 0; i < md->statements.ct; i++) {
        // take the next statement
        RawStatement raw_stmt = buf_get_RawStatement(&md->statements, i);
        // replace the raw args with matching args, if applicable
        int matched_argdef1 = match_macro_argdef(md, raw_stmt.a1);
        if (matched_argdef1 >= 0)
            raw_stmt.a1 = inputs[matched_argdef1];
        int matched_argdef2 = match_macro_argdef(md, raw_stmt.a2);
        if (matched_argdef2 >= 0)
            raw_stmt.a2 = inputs[matched_argdef2];
        int matched_argdef3 = match_macro_argdef(md, raw_stmt.a3);
        if (matched_argdef3 >= 0)
            raw_stmt.a3 = inputs[matched_argdef3];
        AStatement st = read_statement(pst, raw_stmt.mnem, raw_stmt.a1, raw_stmt.a2, raw_stmt.a3);
        buf_push_AStatement(statements, st);
    }
}

void define_macro(ParserState *st, const char *name) {
    MacroDef def;
    def.name = util_strdup(name);
    buf_alloc_MacroArg(&def.args, 4);
    while (peek_token(st).kind != MARK) {
        Token arg = expect_token(st, IDENTIFIER); // expect an argument bind
        MacroArg arg_def;
        arg_def.name = util_strdup(arg.cont);
        if (arg_def.name[0] == 'r') {
            arg_def.type = MACROARG_REG;
        } else if (arg_def.name[0] == 'v') {
            arg_def.type = MACROARG_VAL;
        }
        buf_push_MacroArg(&def.args, arg_def);
    }
    expect_token(st, MARK); // eat the mark
    // interpret the macro body
    buf_alloc_RawStatement(&def.statements, 4);
    while (true) {
        Token next = peek_token(st);
        if (next.kind == MARK && streq(next.cont, "::")) {
            expect_token(st, MARK); // end of macro def
            break;
        }
        // otherwise, we should have an identifier
        Token iden = expect_token(st, IDENTIFIER);
        const char *mnem = iden.cont;
        InstructionInfo info = get_instruction_info(mnem);
        const char *a1 = NULL, *a2 = NULL, *a3 = NULL;
        if (info.type == INSTR_INV) { // not a base instruction
                                      // we don't support referencing macros within macros
            printf("unrecognized mnemonic: %s\n", mnem);
        } else {
            if ((info.type & INSTR_K_R1) > 0) {
                a1 = expect_token(st, IDENTIFIER).cont;
            }
            if ((info.type & INSTR_K_R2) > 0) {
                a2 = expect_token(st, IDENTIFIER).cont;
            }
            if ((info.type & INSTR_K_R3) > 0) {
                a3 = expect_token(st, IDENTIFIER).cont;
            }
        }
        RawStatement raw_stmt = (RawStatement){.mnem = mnem, .a1 = a1, .a2 = a2, .a3 = a3};
        buf_push_RawStatement(&def.statements, raw_stmt);
    }

    buf_push_MacroDef(&st->macros, def); // push the macro
}

MacroDef resolve_macro(ParserState *pst, const char *name) {
    MacroDef md;
    md.name = NULL;
    // find the defined macro
    for (size_t i = 0; i < pst->macros.ct; i++) {
        MacroDef md = buf_get_MacroDef(&pst->macros, i);
        if (streq(md.name, name)) {
            return md;
        }
    }
    // we do not throw an error, the caller will handle that if name is null
    return md;
}

void define_label(ParserState *st, const char *name) {
    char *label_name = util_strdup(name);
    LabelDef ld = {.name = label_name, .offset = st->offset};
    buf_push_LabelDef(&st->labels, ld);
}

int resolve_label(ParserState *st, char *name) {
    // find the defined label
    for (size_t i = 0; i < st->labels.ct; i++) {
        LabelDef lb = buf_get_LabelDef(&st->labels, i);
        if (streq(lb.name, name)) {
            printf("resolved label: %s @ $%04x \n", lb.name, lb.offset);
            return lb.offset;
        }
    }
    printf("ERROR: failed to resolve label %s\n", name);
    return 0; // not resolved
}

void reallocate_program_data(SourceProgram *src, size_t space) {
    if (!src->data) {
        src->data = malloc(sizeof(BYTE) * space);
    } else {
        src->data = realloc(src->data, sizeof(BYTE) * (src->data_size + space));
    }
}

void resolve_value_source(ParserState *pst, ValueSource *vs) {
    if (vs->kind == VS_REF) {
        vs->kind = VS_IMM;
        int label_addr = resolve_label(pst, vs->ref.label);
        vs->val = label_addr + vs->ref.add_offset;
    }
}

void resolve_statements(SourceProgram *src, ParserState *pst) {
    for (size_t i = 0; i < src->statements.ct; i++) {
        AStatement unres_stmt = buf_get_AStatement(&src->statements, i);
        resolve_value_source(pst, &unres_stmt.a1);
        resolve_value_source(pst, &unres_stmt.a2);
        resolve_value_source(pst, &unres_stmt.a3);
        // TODO: resolve all value sources
        buf_set_AStatement(&src->statements, i, unres_stmt);
    }
}

SourceProgram parse(LexResult lexed) {
    ParserState st = {.lexed = &lexed, .token = 0, .cpos = 0, .offset = 0};
    buf_alloc_LabelDef(&st.labels, 16);
    buf_alloc_MacroDef(&st.macros, 16);

    SourceProgram src;
    source_program_init(&src);

    // entry label
    char *entry_label = NULL;

    // emit the entry jump (as nop)
    buf_push_AStatement(&src.statements, IMM_STATEMENT(OP_NOP, 0, 0, 0));
    st.offset += INSTR_SIZE; // push space for entry jump

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
                        printf("ERROR: invalid data (must be even)\n");
                    }
                    pack_len = pack_len / 2;              // divide by two because 0xff = 1 byte
                    BYTE *pack_data = datahex(pack.cont); // convert data from hex
                    // write the pack data to the binary
                    reallocate_program_data(&src, pack_len);
                    // copy the data
                    memcpy(src.data + src.data_size, pack_data, pack_len);
                    free(pack_data); // free decoded data
                    break;
                }
                case QUOT: {
                    Token pack = take_token(&st); // any following token is valid
                    pack_len = strlen(pack.cont);
                    // copy string from token to data
                    reallocate_program_data(&src, pack_len);
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
            if (next.kind == MARK && streq(next.cont, ":")) { // label def (only if single mark)
                expect_token(&st, MARK);                      // eat the mark
                define_label(&st, iden.cont);                 // create label
                break;
            } else if (next.kind == BIND) {   // macro def
                expect_token(&st, BIND);      // eat the bind
                define_macro(&st, iden.cont); // define the macro
                break;
            } else { // instruction
                const char *mnem = iden.cont;
                InstructionInfo info = get_instruction_info(mnem);
                const char *a1 = NULL, *a2 = NULL, *a3 = NULL;
                if (info.type == INSTR_INV) {               // didn't match standard instruction names
                    MacroDef md = resolve_macro(&st, mnem); // check if a matching macro exists
                    if (!md.name) {                         // invalid mnemonic
                        printf("unrecognized mnemonic: %s\n", mnem);
                    } else {
                        // expand the macro
                        expand_macro(&st, &md, &src.statements);
                        break;
                    }
                } else { // fill in arguments
                    if ((info.type & INSTR_K_R1) > 0) {
                        a1 = expect_token(&st, IDENTIFIER).cont;
                    }
                    if ((info.type & INSTR_K_R2) > 0) {
                        a2 = expect_token(&st, IDENTIFIER).cont;
                    }
                    if ((info.type & INSTR_K_R3) > 0) {
                        a3 = expect_token(&st, IDENTIFIER).cont;
                    }
                }

                AStatement raw_stmt = read_statement(&st, iden.cont, a1, a2, a3); // read statement
                buf_push_AStatement(&src.statements, raw_stmt);                   // push statement
            }
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
        src.entry = entry_addr;
    }

    // resolve everything else
    resolve_statements(&src, &st);

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
    Instruction instr = (Instruction){.opcode = st->op, .a1 = st->a1.val, st->a2.val, st->a3.val};
    InstructionInfo info = get_instruction_info_op(st->op);
    if ((info.type & INSTR_K_I1) > 0) {
        // 24-bit constant
        uint32_t val = st->a1.val;
        instr.a1 = (ARG)(val & 0xff);        // lower 8
        instr.a2 = (ARG)((val >> 8) & 0xff); // middle 8
        instr.a3 = (ARG)(val >> 16);         // upper 8
    } else if ((info.type & INSTR_K_I2) > 0) {
        // 16-bit constant
        uint32_t val = st->a2.val;
        instr.a2 = (ARG)(val & 0xff); // lower 8
        instr.a3 = (ARG)(val >> 8);   // upper 8
    } else if ((info.type & INSTR_K_I3) > 0) {
        // 8-bit constant
        uint32_t val = st->a3.val;
        instr.a3 = (ARG)val;
    }
    return instr;
}

CompiledProgram compile_program(SourceProgram src) {
    CompiledProgram cmp;
    compiled_program_init(&cmp);

    // this will only work if src is fully simplified (1 Statement : 1 Instruction)
    cmp.instruction_count = src.statements.ct;
    cmp.instructions = malloc(sizeof(Instruction) * cmp.instruction_count);
    // copy data
    cmp.data = src.data;
    cmp.data_size = src.data_size;

    // step-by-step convert the program
    for (size_t i = 0; i < src.statements.ct; i++) {
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
        free(src.data); // free data
        src.data = NULL;
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

typedef struct {
    bool valid_magic;
    // uint16_t entry;
    uint16_t code_size;
    uint16_t data_size;
    size_t decode_offset;
} RGHeader;

void write_compiled_program(FILE *ouf, CompiledProgram cmp) {
    char w = '\0';

    // write header
    RGHeader head;
    head.valid_magic = true;
    head.code_size = cmp.instruction_count * INSTR_SIZE;
    head.data_size = cmp.data_size;
    head.decode_offset = HEADER_SIZE;
    const char *RG_MAGIC = "rg";
    fputs(RG_MAGIC, ouf); // magic
    // fwrite(&cmp.entry, sizeof(cmp.entry), 1, ouf); // entrypoint
    fwrite(&head.code_size, sizeof(head.code_size), 1, ouf); // code size
    fwrite(&head.data_size, sizeof(head.data_size), 1, ouf); // data size
    printf("head[%02d] \n", HEADER_SIZE);

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
    int offset = cmp.data_size;
    for (size_t i = 0; i < cmp.instruction_count; i++) {
        Instruction in = cmp.instructions[i];
        if (rich)
            printf("%04x ", offset);
        dump_instruction(in, rich);
        offset += INSTR_SIZE;
    }
}

/* #endregion */

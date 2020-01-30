#include "asm.h"
#include "asm_ext.h"
#include "util.h"
#include <stdio.h>

typedef struct {
    bool compat;
} AssemblerOptions;

int main(int argc, char **argv) {
    printf("[REGULAR_ad] assembler\n");
    if (argc < 3) {
        printf("usage: asm <in> <out> --opts\n");
    }
    char *in_file = argv[1];
    char *out_file = argv[2];

    AssemblerOptions options = {
        .compat = false,
    };

    for (int i = 3; i < argc; i++) {
        char *flg = argv[i];
        if (streq(flg, "--compat")) {
            options.compat = true;
            printf("enabling compatibility mode\n");
        }
    }

    // open input file
    FILE *inf_fp = fopen(in_file, "rb");
    if (inf_fp == NULL) {
        fprintf(stderr, "cannot open input file\n");
        return 1;
    }

    // open output file
    FILE *ouf_fp = fopen(out_file, "wb");
    if (ouf_fp == NULL) {
        fprintf(stderr, "cannot open output file\n");
        return 1;
    }

    FileReadResult inf_read = util_read_file_contents(inf_fp);
    fclose(inf_fp);

    // run lexer on the input text
    LexResult lex_result = lex(inf_read.content, inf_read.size);
    printf("== TOKENS ==\n");
    for (int i = 0; i < lex_result.token_count; i++) {
        Token tok = lex_result.tokens[i];
        // print token
        printf("%4d TOK: %10s [%3d]\n", i, tok.cont, (int)tok.kind);
    }
    // parse the tokens into a program
    printf("== PARSE ==\n");
    // parse program with utility instructions
    Program prg_hl = parse(lex_result);
    if (prg_hl.status != 0) { // unsuccessful program
        printf("assembly pass 0 failed [%d]\n", prg_hl.status);
        return 2;
    }

    // dump the pass 0 program
    printf("== DUMP [pre] ==\n");
    dump_program(prg_hl);

    // compile pseudo-instructions - pass 1
    Program prg_pass1 = compile_pseudo(prg_hl);
    Program prg_out = compile_pseudo(prg_pass1);
    free_program(prg_hl, false);
    free_program(prg_pass1, false);
    
    // dump the finished program
    printf("== DUMP [fin] ==\n");
    dump_program(prg_out);
    // write out the program to binary
    printf("== WRITE ==\n");
    write_program(ouf_fp, prg_out, options.compat);

    // clean up
    free(inf_read.content);
    free_lex_result(lex_result);
    free_program(prg_out, true);

    fclose(ouf_fp); // close output file

    return 0;
}

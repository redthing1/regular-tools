#include "asm.h"
#include "asm_ext.h"
#include "util.h"
#include <stdio.h>

typedef struct {
    bool compat;
    bool debug_tokens;
} AssemblerOptions;

int main(int argc, char **argv) {
    printf("[REGULAR_ad] assembler v2.0\n");
    if (argc < 3) {
        printf("usage: asm <in> <out> --opts\n");
    }
    char *in_file = argv[1];
    char *out_file = argv[2];

    AssemblerOptions options = {
        .compat = false,
        .debug_tokens = false,
    };

    for (int i = 3; i < argc; i++) {
        char *flg = argv[i];
        if (streq(flg, "--compat")) {
            options.compat = true;
            printf("enabling compatibility mode\n");
        }
        if (streq(flg, "--debug-tokens")) {
            options.debug_tokens = true;
            printf("token dumping enabled\n");
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
    if (options.debug_tokens) {
        printf("== TOKENS ==\n");
        for (int i = 0; i < lex_result.token_count; i++) {
            Token tok = buf_get_Token(&lex_result.tokens, i);
            // print token
            printf("%4d TOK: %10s [%3d]\n", i, tok.cont, (int)tok.kind);
        }
    }
    // parse the tokens into a program
    printf("== PARSE ==\n");
    // parse program with utility instructions
    SourceProgram source = parse(lex_result);
    if (source.status != 0) { // unsuccessful program
        printf("assembly pass 0 failed [%d]\n", source.status);
        return 2;
    }

    // dump the pass 0 program
    printf("== DUMP [src] ==\n");
    dump_source_program(source);

    // simplify program
    SourceProgram final = simplify_pseudo_2pass(source);
    free_source_program(source, false);

    CompiledProgram compiled = compile_program(final);

    // dump the compiled program
    printf("== DUMP [cmp] ==\n");
    dump_compiled_program(compiled);

    // write out the program to binary
    printf("== WRITE ==\n");
    write_program(ouf_fp, compiled);

    // clean up
    free(inf_read.content);
    free_lex_result(lex_result);
    free_source_program(final, true);

    fclose(ouf_fp); // close output file

    return 0;
}

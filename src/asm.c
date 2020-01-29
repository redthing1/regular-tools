#include <stdio.h>
#include "asm.h"
#include "util.h"

int main(int argc, char **argv) {
    printf("[REGULAR_ad] assembler\n");
    if (argc < 2) {
        printf("usage: asm --opts <in> <out>\n");
    }
    char *in_file = argv[1];
    char *out_file = argv[2];

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
    for (int i = 0; i < lex_result.token_count; i++) {
        Token tok = lex_result.tokens[i];
        // print token
        printf("%4d TOK: %10s [%3d]\n", i, tok.cont, (int) tok.kind);
    }
    // parse the tokens into a program
    Program prg = parse(lex_result);
    if (prg.status == 0) { // successful program
        // TODO: write out the program to binary
        
    }


    // clean up
    free(inf_read.content);
    free_lex_result(lex_result);
    free_program(prg);
    
    fclose(ouf_fp); // close output file

    return 0;
}

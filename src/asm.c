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
    FILE *ouf_fp = fopen(out_file, "rb");
    if (ouf_fp == NULL) {
        fprintf(stderr, "cannot open output file\n");
        return 1;
    }

    FileReadResult inf_read = util_read_file_contents(inf_fp);
    fclose(inf_fp);
    lex(inf_read.content, inf_read.size);

    fclose(ouf_fp);

    return 0;
}

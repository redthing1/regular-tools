#include <stdio.h>

int main(int argc, char **argv) {
    printf("[REGULAR_ad] assembler\n");
    if (argc < 2) {
        printf("usage: asm --opts <in> <out>");
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

    fclose(inf_fp);
    fclose(ouf_fp);

    return 0;
}

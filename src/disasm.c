#include "disasm.h"
#include "asm.h"
#include "util.h"
#include <stdio.h>

typedef struct {
    bool raw;
} DisasmOptions;

int main(int argc, char **argv) {
    printf("[REGULAR_ad] disassembler v1.0\n");
    if (argc < 2) {
        printf("usage: disasm <in> --args\n");
    }

    char *in_file = argv[1];

    DisasmOptions options = {
        .raw = false,
    };

    for (int i = 2; i < argc; i++) {
        char *flg = argv[i];
        if (streq(flg, "--raw")) {
            options.raw = true;
            printf("enabling RAW dump mode\n");
        }
    }

    // open input file
    FILE *inf_fp = fopen(in_file, "rb");
    if (inf_fp == NULL) {
        fprintf(stderr, "cannot open input file\n");
        return 1;
    }

    FileReadResult inf_read = util_read_file_contents(inf_fp);
    fclose(inf_fp);

    DecoderResult decode_result = decode_compiled_program(inf_read.content, inf_read.size);
    if (decode_result.status == 0) { // successful decode
        printf("== DUMP ==\n");
        dump_compiled_program(decode_result.cmp, !options.raw);
    } else {
        printf("program decode FAILED.\n");
        return 1;
    }

    // clean up
    free(inf_read.content);
    free_compiled_program(decode_result.cmp);

    return 0;
}

#include "emu.h"
#include "asm.h"
#include "disasm.h"
#include "util.h"
#include <stdio.h>

typedef struct {
    bool step;
} EmuOptions;

int main(int argc, char **argv) {
    printf("[REGULAR_ad] emulator\n");
    if (argc < 2) {
        printf("usage: emu <in> --flags\n");
    }

    char *in_file = argv[1];

    EmuOptions options = {
        .step = false,
    };

    for (int i = 2; i < argc; i++) {
        char *flg = argv[i];
        if (streq(flg, "--step")) {
            options.step = true;
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

    EmulatorState *emu_st = emu_init();
    emu_st->onestep = options.step;
    // copy program to offset 0
    RGHeader hd = emu_load(emu_st, 0, inf_read.content, inf_read.size);
    emu_run(emu_st, hd.entry);

    // clean up
    emu_free(emu_st);
    free(inf_read.content);

    return 0;
}

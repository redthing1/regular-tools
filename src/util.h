/*
util.h
random utilities
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *content;
    size_t size;
} FileReadResult;

/**
 * Takes a file pointer and reads the full contents of the file.
 * The caller is responsible for cleaning up the file pointer and buffer.
 */
FileReadResult util_read_file_contents(FILE *fp) {
    // get the file size
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    // allocate buffer with file size and nullterm
    char *buffer = malloc((size + 1) * sizeof(*buffer));

    fread(buffer, size, 1, fp); // read chunk to buffer
    buffer[size] = '\0';        // add null terminator

    FileReadResult res = {.content = buffer, .size = size};
    return res;
}

bool streq(char *s1, char *s2) { return strcmp(s1, s2) == 0; }

// https://stackoverflow.com/questions/21133701/is-there-any-function-in-the-c-language-which-can-convert_base-base-of-decimal-number/21134322#21134322
int convert_dec_to(int val, int base) {
    if (val == 0 || base == 10)
        return val;
    return (val % base) + 10 * convert_dec_to(val / base, base);
}

void util_pause() {
    size_t pause_bufsize = 256;
    char pause_buf[pause_bufsize];
    fgets(pause_buf, pause_bufsize, stdin);
}

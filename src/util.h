/*
util.h
random utilities
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char* content;
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
    char* buffer = malloc((size + 1) * sizeof(*buffer));

    fread(buffer, size, 1, fp); // read chunk to buffer
    buffer[size] = '\0'; // add null terminator

    FileReadResult res = {.content = buffer, .size = size};
    return res;
}

bool streq(char* s1, char* s2) {
    return strcmp(s1, s2) == 0;
}

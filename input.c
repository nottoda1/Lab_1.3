#include <stdio.h>
#include <stdlib.h>
#include "input.h"
char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    char *res = NULL;
    if (f) {
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        rewind(f);
        res = (char *)malloc(size + 1);
        if (res) {
            fread(res, 1, size, f);
            res[size] = '\0';
        }
        fclose(f);
    }
    return res;
}
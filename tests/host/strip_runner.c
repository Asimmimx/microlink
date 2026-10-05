#include <stdio.h>
#include <stdlib.h>
#include "ml_json_strip.h"
int main(int argc, char **argv) {
    (void)argc;
    static const char *const keys[] = {"Hostinfo","CapMap","PacketFilter","Drop",NULL};
    FILE *f = fopen(argv[1], "rb"); fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *b = malloc(n + 1); fread(b, 1, n, f); fclose(f);
    size_t m = ml_json_strip_keys(b, (size_t)n, keys);
    f = fopen(argv[2], "wb"); fwrite(b, 1, m, f); fclose(f);
    return 0;
}

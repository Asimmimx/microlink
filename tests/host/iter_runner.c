#include <stdio.h>
#include <stdlib.h>
#include "ml_json_strip.h"
int main(int argc, char **argv) {
    (void)argc;
    FILE *f = fopen(argv[1], "rb"); fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *b = malloc(n + 1); fread(b, 1, n, f); fclose(f);
    FILE *o = fopen(argv[2], "wb");
    size_t pos = 0, ks, kl, vs, vl;
    while (pos < (size_t)n && b[pos] != '{') pos++;
    while (ml_json_next_member(b, n, &pos, &ks, &kl, &vs, &vl)) {
        fwrite(b + ks, 1, kl, o); fputc('\x01', o); fwrite(b + vs, 1, vl, o); fputc('\x02', o);
    }
    fclose(o);
    return 0;
}

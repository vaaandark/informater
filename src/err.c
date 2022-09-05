#include "err.h"

void fmt_panic(int line, int column, const char *token,
        const char *msg) {
    fprintf(stderr, "\033[1;033minformater:\033[0m @ %d:%d"
            " \033[1;31mfatal\033[0m: '%s', %s\n",
            line, column, token, msg);
    abort();
}

void fmt_panic_with_expect(int line, int column,
        const char *token, const char *expect, const char *msg) {
    fprintf(stderr, "\033[1;033minformater:\033[0m @ %d:%d"
            " \033[1;31mfatal\033[0m: '%s', expect '%s', %s\n",
            line, column, token, expect, msg);
    abort();
}

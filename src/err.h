#ifndef ERR_H_
#define ERR_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern void fmt_panic(int line, int column,
        const char *token, const char *msg);

extern void fmt_panic_with_expect(int line, int column,
        const char *token, const char *expect, const char *msg);

#endif

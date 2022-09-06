#include <stdio.h>
char ch; int afunc(int a, float b); int main(void) { int i; int a; float array[10 + 1], f; array[1] = 1.f; a = 10; while (a > 0) { // 这是一个 while 循环
        a -= 1;
    }
    // 连续的三行注释
    /* 这是另一种风格的注释 */
    // 这是一个 for 循环
    for (i = 0; i < 10; i += 1) { if (i && 0 == i % 2) { puts("even"); continue; } else { puts("odd"); break; } printf("hello * %d", i); printf("%f\n", array[i]); } }
int afunc(int a, float b) { return 0; }

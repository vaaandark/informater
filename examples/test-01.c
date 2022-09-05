#include <stdio.h>
int main(void) {
    int i;
    float array[10], f;
    for (i = 0; i < 10; i += 1) {
        printf("hello * %d", i);
        printf("%f\n", array[i]);
    }
}

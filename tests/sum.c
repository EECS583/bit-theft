#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void sum(uint64_t *result, bool a, bool b, uint64_t c, uint64_t d, uint64_t e,
         uint64_t g, uint64_t h) {
    if (c == 0)
        *result = h;
    else {
        uint64_t left = 0, right = 0;
        if (a)
            sum(&left, !a, !b, c - 1, c + d, c + d + e, c + d + e + g,
                c + d + e + g + h);
        if (b)
            sum(&right, !a, !b, c - 1, c + d, c + d + e, c + d + e + g,
                c + d + e + g + h + h);
        *result = left + right;
    }
}

int main() {
    srand((unsigned int)time(NULL));
    uint64_t everything = 0;
    for (unsigned int i = 0; i < 1000000; i++) {
        uint64_t sigma = 0;
        sum(&sigma, true, false, 1000, (uint64_t)rand(), (uint64_t)rand(),
            (uint64_t)rand(), (uint64_t)rand());
        everything += sigma;
    }
    printf("The strange sum is: %ld\n", everything);
    return 0;
}

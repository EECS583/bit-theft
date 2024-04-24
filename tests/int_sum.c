#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t sum(uint32_t a, uint32_t b, uint64_t c, uint64_t d, uint64_t e,
             uint64_t f, uint64_t g) {
    if (a == 0)
        return g;
    else
        return sum(a - 1, a + b, a + b + c, a + b + c + d, a + b + c + d + e,
                   a + b + c + d + e + f, a + b + c + d + e + f + g) +
               sum(a - 1, a + b, a + b + c, a + b + c + d, a + b + c + d + e,
                   a + b + c + d + e + f, a + b + c + d + e + f + g);
    ;
}

int main() {
    srand(42);
    uint64_t everything = 0;
    for (unsigned int i = 0; i < 65536 * 512; i++) {
        everything += sum((uint32_t)rand() % 8 + 1, (uint32_t)rand() % 8 + 1,
                          (uint64_t)rand(), (uint64_t)rand(), (uint64_t)rand(),
                          (uint64_t)rand(), (uint64_t)rand());
    }
    printf("The strange sum is: %lu\n", everything);
    return 0;
}

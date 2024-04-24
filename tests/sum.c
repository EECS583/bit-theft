#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint64_t sum(uint32_t a, uint32_t b, uint64_t c, uint64_t d, uint64_t e,
             uint64_t g, uint64_t h) {
    if (a == 0)
        return h;
    else
        return sum(a - 1, a + b, a + b + c, a + b + c + d, a + b + c + d + e,
                   a + b + c + d + e + g, a + b + c + d + e + g + h);
}

int main() {
    srand((unsigned int)time(NULL));
    uint64_t sigma =
        sum(100, (uint32_t)rand(), (uint64_t)rand(), (uint64_t)rand(),
            (uint64_t)rand(), (uint64_t)rand(), (uint64_t)rand());
    printf("The strange sum is: %ld\n", sigma);
    return 0;
}

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int getCounter(int *counters, int x, int y, int N) {
    return counters[x * N + y];
}

void setCounter(int *counters, int x, int y, int N) { counters[x * N + y]++; }

int result = 0;
void walk(int *counters, int x, int y, int N, int depth, bool goVerticle,
          bool goHorizontal, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
          uint64_t a5, uint64_t a6) {
    x = x % N;
    y = y % N;
    setCounter(counters, x, y, N);
    if (depth == 128) {
        result += x + y;
        // Print out the entire counter array
        // printf("\n=====================\n");
        // for (int i = 0; i < N; i++) {
        //     for (int j = 0; j < N; j++) {
        //         printf("%d ", getCounter(counters, i, j, N));
        //     }
        //     printf("\n");
        // }

        return;
    }
    bool nextGoVerticle = x % 2 == 0;
    bool nextGoHorizontal = y % 2 == 0 || x % 2 != 0;
    // If go verticle, randomly choose to go up or down
    if (goVerticle) {
        rand() ? walk(counters, x - 1, y, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal, a1 + a2, a3 + a4, a5 + a6, a1 + a2,
                      a3 + a4, a5 + a6)
               : walk(counters, x + 1, y, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal, a1 + a2, a3 + a4, a5 + a6, a1 + a2,
                      a3 + a4, a5 + a6);
    }
    // if go horizontal, randomly choose to go left or right
    if (goHorizontal) {
        rand() ? walk(counters, x, y - 1, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal, a1 + a2, a3 + a4, a5 + a6, a1 + a2,
                      a3 + a4, a5 + a6)
               : walk(counters, x, y + 1, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal, a1 + a2, a3 + a4, a5 + a6, a1 + a2,
                      a3 + a4, a5 + a6);
    }
}

int main() {
    srand(10);
    const int N = 2;
    int *counters = (int *)malloc(N * N * sizeof(int));
    walk(counters, N / 2, N / 2, N, 0, 1, 1, 1, 2, 3, 4, 5, 6);
    printf("%d\n", result);
}

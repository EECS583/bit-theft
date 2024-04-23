#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int getCounter(int *counters, int x, int y, int N) {
    return counters[x * N + y];
}

void setCounter(int *counters, int x, int y, int N) { counters[x * N + y]++; }

void walk(int *counters, int x, int y, int N, int depth, bool goVerticle,
          bool goHorizontal) {
    x = x % N;
    y = y % N;
    setCounter(counters, x, y, N);
    if (depth == 10) {
        // Print out the entire counter array
        printf("\n=====================\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", getCounter(counters, i, j, N));
            }
            printf("\n");
        }

        return;
    }
    bool nextGoVerticle = x % 2 == 0;
    bool nextGoHorizontal = y % 2 == 0 || x % 2 != 0;
    // If go verticle, randomly choose to go up or down
    if (goVerticle) {
        rand() ? walk(counters, x - 1, y, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal)
               : walk(counters, x + 1, y, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal);
    }
    // if go horizontal, randomly choose to go left or right
    if (goHorizontal) {
        rand() ? walk(counters, x, y - 1, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal)
               : walk(counters, x, y + 1, N, depth + 1, nextGoVerticle,
                      nextGoHorizontal);
    }
}

int main() {
    srand(10);
    int N = 0;
    scanf("%d", &N);
    int *counters = (int *)malloc(N * N * sizeof(int));
    walk(counters, N / 2, N / 2, N, 0, 1, 1);
}

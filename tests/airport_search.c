#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool getVisited(bool *visited, int64_t x, int64_t y, bool isArrivalLayer, int64_t H, int64_t W) {
    return visited[isArrivalLayer * H * W + x * W + y];
}

void setVisited(bool *visited, int64_t x, int64_t y, bool isArrivalLayer, int64_t H, int64_t W) {
    visited[isArrivalLayer * H * W + x * W + y] = 1;
}

int getMap(int *map, int64_t x, int64_t y, int64_t H, int64_t W) { return map[x * W + y]; }

void setMap(int *map, int64_t x, int64_t y, int64_t H, int64_t W, int value) {
    map[x * W + y] = value;
}

// Map value 0 means empty, 1 means obstacle, 2 means elevator, 3 means arrival
bool find(int *map, bool *visited, int64_t x, int64_t y, bool isArrivalLayer,
          int64_t H, int64_t W, bool depth) {
    if (x < 0 || x >= H || y < 0 || y >= W ||
        getVisited(visited, x, y, isArrivalLayer, H, W) ||
        getMap(map, x, y, H, W) == 1) {
        return false;
    }
    if (getMap(map, x, y, H, W) == 3 && isArrivalLayer) {
        printf("Path found with depth %d\n", depth);
        return true;
    }
    setVisited(visited, x, y, isArrivalLayer, H, W);
    if (getMap(map, x, y, H, W) == 2) {
        if (find(map, visited, x, y, !isArrivalLayer, H, W, !depth)) {
            return true;
        }
    }
    return find(map, visited, x - 1, y, isArrivalLayer, H, W, !depth) ||
           find(map, visited, x + 1, y, isArrivalLayer, H, W, !depth) ||
           find(map, visited, x, y - 1, isArrivalLayer, H, W, !depth) ||
           find(map, visited, x, y + 1, isArrivalLayer, H, W, !depth);
}

int main() {
    const unsigned int N = 128;
    int *map = (int *)calloc(N * N, sizeof(int));
    for (unsigned int i = 0; i < 10000; i++) {
        bool *visited = (bool *)calloc(2 * N * N, sizeof(bool));
        if (!find(map, visited, 0, 0, true, N, N, 0)) {
            // printf("No path found\n");
        }
        free(visited);
    }
    free(map);
    return 0;
}

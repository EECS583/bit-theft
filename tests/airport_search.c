#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>

bool getVisited(bool *visited, int x, int y, bool isArrivalLayer, int N) {
    return visited[isArrivalLayer * N * N + x * N + y];
}

void setVisited(bool *visited, int x, int y, bool isArrivalLayer, int N) {
    visited[isArrivalLayer * N * N + x * N + y] = 1;
}

int getMap(int *map, int x, int y, int N) { return map[x * N + y]; }

void setMap(int *map, int x, int y, int N, int value) {
    map[x * N + y] = value;
}

// Map value 0 means empty, 1 means obstacle, 2 means elevator, 3 means arrival
bool find(int *map, bool *visited, int x, int y, bool isArrivalLayer, int N,
          int depth) {
    if (x < 0 || x >= N || y < 0 || y >= N ||
        getVisited(visited, x, y, isArrivalLayer, N) ||
        getMap(map, x, y, N) == 1) {
        return false;
    }
    if (getMap(map, x, y, N) == 3 && isArrivalLayer) {
        printf("Path found with depth %d\n", depth);
        return true;
    }
    setVisited(visited, x, y, isArrivalLayer, N);
    if (getMap(map, x, y, N) == 2) {
        if (find(map, visited, x, y, !isArrivalLayer, N, depth + 1)) {
            return true;
        }
    }
    return find(map, visited, x - 1, y, isArrivalLayer, N, depth + 1) ||
           find(map, visited, x + 1, y, isArrivalLayer, N, depth + 1) ||
           find(map, visited, x, y - 1, isArrivalLayer, N, depth + 1) ||
           find(map, visited, x, y + 1, isArrivalLayer, N, depth + 1);
}

int main() {
    int N = 0;
    scanf("%d", &N);
    int *map = (int *)malloc(N * N * sizeof(int));
    bool *visited = (bool *)malloc(2 * N * N * sizeof(bool));
    memset(visited, 0, 2 * N * N * sizeof(bool));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            scanf("%d", &map[i * N + j]);
        }
    }
    if (!find(map, visited, 0, 0, true, N, 0)) {
        printf("No path found\n");
    }
    return 0;
}

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct List {
    bool value;
    struct List *next;
} List;

static uint64_t __attribute__((noinline))
count_rising_edge(const List *list, uint64_t rising_edges, bool last_value) {
    if (list) {
        rising_edges = rising_edges + ((!last_value && list->value) ? 1 : 0);
        return count_rising_edge(list->next, rising_edges, list->value);
    } else {
        return rising_edges;
    }
}

int main(int argc, char *argv[]);
int main(int argc, char *argv[]) {
    List first = {.value = true, .next = NULL};
    List second = {.value = false, .next = NULL};
    List third = {.value = true, .next = NULL};
    List fourth = {.value = true, .next = NULL};
    first.next = &second;
    second.next = &third;
    third.next = &fourth;
    uint64_t rising_edges = count_rising_edge(&first, 0, true);
    assert(rising_edges == 1);
    printf("Rising edges in the list: %ld\n", rising_edges);
    return 0;
}

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct List {
    bool value;
    struct List *next;
} List;

static List *generate_random_list(size_t n) {
    if (n == 0)
        return NULL;
    List *node = (List *)calloc(1, sizeof(List));
    node->value = rand() % 2 == 1;
    node->next = generate_random_list(n - 1);
    return node;
}

static void free_list(List *list) {
    if (list == NULL)
        return;
    free_list(list->next);
    free(list);
}

static size_t count_rising_edge(const List *list, size_t rising_edges,
                                bool last_value) {
    return list ? count_rising_edge(list->next,
                                    rising_edges +
                                        ((!last_value && list->value) ? 1 : 0),
                                    list->value)
                : rising_edges;
}

int main() {
    srand((unsigned int)time(NULL));
    List first = {.value = true, .next = NULL};
    List second = {.value = false, .next = NULL};
    List third = {.value = true, .next = NULL};
    List fourth = {.value = true, .next = NULL};
    first.next = &second;
    second.next = &third;
    third.next = &fourth;
    size_t rising_edges = count_rising_edge(&first, 0, true);
    assert(rising_edges == 1);
    printf("Rising edges in the list: %ld\n", rising_edges);
    int n = rand() % 100;
    List *list = generate_random_list((size_t)(n < 0 ? -n : n));
    printf("Rising edges in random list: %ld\n",
           count_rising_edge(list, 0, true));
    free_list(list);
    return 0;
}

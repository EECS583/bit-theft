#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct List {
    bool value;
    struct List *next;
} List;

static void print_list_helper(List *list);
static List *interleave_lists_helper(List *first, List *second, bool target);

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

static List *interleave_lists(List *first, List *second) {
    if (!first && !second) {
        return NULL;
    }
    List *node = (List *)calloc(1, sizeof(List));
    if (!first) { // if list 1 is empty, start inserting from 2
        node->value = second->value;
        node->next = interleave_lists_helper(first, second->next, false);
    } else { // if list 1 is non-empty, start inserting from 1
        node->value = first->value;
        node->next = interleave_lists_helper(first->next, second, false);
    }
    return node;
}

static List *interleave_lists_helper(List *first, List *second, bool target) {
    if (!first && !second) {
        return NULL;
    }
    List *node = (List *)calloc(1, sizeof(List));
    if (!first) { // if list 1 is empty, keep inserting from 2 from now on
        node->value = second->value;
        node->next = interleave_lists_helper(NULL, second->next, false);
    } else if (!second) { // if list 2 is empty, keep inserting from 1 from now
                          // on
        node->value = first->value;
        node->next = interleave_lists_helper(first->next, NULL, true);
    } else {          // if both lists are non-empty
        if (target) { // insert from list 1, then 2 next time
            node->value = first->value;
            node->next = interleave_lists_helper(first->next, second, false);
        } else { // insert from list 2, then 1 next time
            node->value = second->value;
            node->next = interleave_lists_helper(first, second->next, true);
        }
    }
    return node;
}

static void print_list(List *list) {
    if (list == NULL)
        return;
    printf("%d ", list->value);
    print_list_helper(list->next);
    printf("\n");
}

static void print_list_helper(List *list) {
    if (list == NULL)
        return;
    printf("%d ", list->value);
    print_list_helper(list->next);
}

static size_t get_random_size() { return (size_t)rand() % 10 + 5; }

int main() {
    srand((unsigned int)time(NULL));
    size_t n = get_random_size();
    List *first = generate_random_list(n);
    print_list(first);
    n = get_random_size();
    List *second = generate_random_list(n);
    print_list(second);
    List *result = interleave_lists(first, second);
    print_list(result);
    free_list(first);
    free_list(second);
    free_list(result);
}

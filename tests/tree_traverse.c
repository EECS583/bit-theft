#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Tree {
    struct Tree *left;
    struct Tree *right;
    bool value;
} Tree;

static Tree *generate_random_tree(size_t n) {
    if (n == 0)
        return NULL;
    Tree *tree = (Tree *)calloc(1, sizeof(Tree));
    tree->value = rand() % 2;
    tree->left = generate_random_tree(n - 1);
    tree->right = generate_random_tree(n - 1);
    return tree;
}

static void free_random_tree(Tree *tree) {
    if (tree == NULL)
        return;
    free_random_tree(tree->left);
    free_random_tree(tree->right);
    free(tree);
}

static size_t count_falling(const Tree *tree, bool parent) {
    if (tree == NULL)
        return 0;
    else
        return count_falling(tree->left, tree->value) +
               count_falling(tree->right, tree->value) +
               ((!parent && tree->value) ? 1 : 0);
}

int main() {
    srand((unsigned int)time(NULL));
    Tree *tree = generate_random_tree((size_t)(rand() % 5));
    printf("Rising edges in random list: %ld\n", count_falling(tree, true));
    free_random_tree(tree);
    return 0;
}

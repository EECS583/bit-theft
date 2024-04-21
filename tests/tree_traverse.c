#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Tree {
    bool value;
    struct Tree *left;
    struct Tree *right;
} Tree;

static Tree *generate_random_tree(size_t n) {
    if (n == 0)
        return NULL;
    Tree *tree = (Tree *)calloc(1, sizeof(Tree));
    int seed = rand() % 8;
    tree->value = seed >> 2;
    switch (seed & 0b11) {
    case 0b10:
        tree->left = generate_random_tree(n - 1);
        tree->right = NULL;
        break;
    case 0b01:
        tree->left = NULL;
        tree->right = generate_random_tree(n - 1);
        break;
    default:
        tree->left = generate_random_tree((n - 1) / 2);
        tree->right = generate_random_tree(n - (n + 1) / 2);
        break;
    }
    return tree;
}

static void free_tree(Tree *tree) {
    if (tree == NULL)
        return;
    free_tree(tree->left);
    free_tree(tree->right);
    free(tree);
}

static bool is_alternating(const Tree *tree, bool last) {
    return tree == NULL ||
           (last != tree->value && is_alternating(tree->left, tree->value) &&
            is_alternating(tree->right, tree->value));
}

bool is_alternating_ref(const Tree *tree, bool last) {
    return tree == NULL ||
           (last != tree->value && is_alternating(tree->left, tree->value) &&
            is_alternating(tree->right, tree->value));
}

int main() {
    srand((unsigned int)time(NULL));
    Tree *tree = generate_random_tree((size_t)(rand() % 100));
    assert(is_alternating(tree, true) == is_alternating_ref(tree, true));
    printf("Is tree alternating: %s\n",
           is_alternating(tree, true) ? "true" : "false");
    free_tree(tree);
    return 0;
}

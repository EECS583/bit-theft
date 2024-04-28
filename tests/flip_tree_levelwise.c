#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct TreeNode
{
    int value;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

TreeNode *newTreeNode(int value)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void swapChildren(TreeNode *node)
{
    TreeNode *temp = node->left;
    node->left = node->right;
    node->right = temp;
}

void randomly_generate_tree(TreeNode *root, TreeNode *root_ref, int depth)
{
    if (depth == 0)
        return;

    if (rand() % 2)
    {
        // 50% chance to add a left child
        int leftValue = rand() % 100;
        root->left = newTreeNode(leftValue);
        root_ref->left = newTreeNode(leftValue);
        randomly_generate_tree(root->left, root_ref->left, depth - 1);
    }
    if (rand() % 2)
    {
        // 50% chance to add a right child
        int rightValue = rand() % 100;
        root->right = newTreeNode(rightValue);
        root_ref->right = newTreeNode(rightValue);
        randomly_generate_tree(root->right, root_ref->right, depth - 1);
    }
}

void flipTree(TreeNode *node, bool flip)
{
    if (!node)
        return;

    if (flip)
    {
        swapChildren(node);
    }

    flipTree(node->left, !flip);
    flipTree(node->right, !flip);
}

__attribute__((optnone)) void
flipTree_ref(TreeNode *node, bool flip)
{
    if (!node)
        return;

    if (flip)
    {
        swapChildren(node);
    }

    flipTree(node->left, !flip);
    flipTree(node->right, !flip);
}

void printInOrder(TreeNode *node)
{
    if (node == NULL)
        return;
    printInOrder(node->left);
    printf("%d ", node->value);
    printInOrder(node->right);
}

void freeTree(TreeNode *node)
{
    if (node == NULL)
        return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main()
{
    TreeNode *root = newTreeNode(1);
    root->left = newTreeNode(2);
    root->right = newTreeNode(3);
    root->left->left = newTreeNode(4);
    root->left->right = newTreeNode(5);
    root->right->left = newTreeNode(6);
    root->right->right = newTreeNode(7);

    TreeNode *root_ref = newTreeNode(1);
    root_ref->left = newTreeNode(2);
    root_ref->right = newTreeNode(3);
    root_ref->left->left = newTreeNode(4);
    root_ref->left->right = newTreeNode(5);
    root_ref->right->left = newTreeNode(6);
    root_ref->right->right = newTreeNode(7);

    printf("Original tree in-order      : ");
    printInOrder(root);
    printf("\n");

    flipTree(root, true);
    flipTree_ref(root_ref, true);

    printf("Flipped tree in-order       : ");
    printInOrder(root);
    printf("\n");
    printf("Flipped tree in-order (ref) : ");
    printInOrder(root_ref);
    printf("\n");

    freeTree(root);
    freeTree(root_ref);

    //*** Random Testcase ***//
    printf("\n>>>>>> Random Testcase <<<<<<\n");

    srand(time(NULL));
    TreeNode *rand_root = newTreeNode(1);
    TreeNode *rand_root_ref = newTreeNode(1);

    randomly_generate_tree(rand_root, rand_root_ref, 20);

    printf("Original tree in-order      : ");
    printInOrder(rand_root);
    printf("\n");

    flipTree(root, true);
    flipTree_ref(rand_root_ref, true);

    printf("Flipped tree in-order       : ");
    printInOrder(rand_root);
    printf("\n");
    printf("Flipped tree in-order (ref) : ");
    printInOrder(rand_root_ref);
    printf("\n");

    freeTree(rand_root);
    freeTree(rand_root_ref);

    return 0;
}

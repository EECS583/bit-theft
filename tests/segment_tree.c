#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX 1000  // Maximum array size (adjust as needed)

typedef struct {
    int sum;
    int lazy;
} SegmentTree;

void build(SegmentTree* tree, int* arr, int node, int start, int end) {
    if (start == end) {
        // Leaf node will have a single element
        tree[node].sum = arr[start];
        tree[node].lazy = 0;
    } else {
        int mid = (start + end) / 2;
        build(tree, arr, 2 * node + 1, start, mid);
        build(tree, arr, 2 * node + 2, mid + 1, end);
        tree[node].sum = tree[2 * node + 1].sum + tree[2 * node + 2].sum;
        tree[node].lazy = 0;
    }
}

void updateRange(SegmentTree* tree, int node, int start, int end, int l, int r, bool val) {
    if (tree[node].lazy != 0) {
        // This node needs to be updated
        tree[node].sum += (end - start + 1) * tree[node].lazy;
        if (start != end) {
            tree[2 * node + 1].lazy += tree[node].lazy;
            tree[2 * node + 2].lazy += tree[node].lazy;
        }
        tree[node].lazy = 0;
    }
    if (start > end || start > r || end < l)
        return;

    if (start >= l && end <= r) {
        // Segment is fully within the range
        int delta = (val * 2 - 1);
        tree[node].sum += (end - start + 1) * delta;
        if (start != end) {
            tree[2 * node + 1].lazy += delta;
            tree[2 * node + 2].lazy += delta;
        }
        return;
    }

    int mid = (start + end) / 2;
    updateRange(tree, 2 * node + 1, start, mid, l, r, val);
    updateRange(tree, 2 * node + 2, mid + 1, end, l, r, val);
    tree[node].sum = tree[2 * node + 1].sum + tree[2 * node + 2].sum;
}

int queryRange(SegmentTree* tree, int node, int start, int end, int l, int r) {
    if (start > end || start > r || end < l)
        return 0; // Out of range

    if (tree[node].lazy != 0) {
        // This node needs to be updated
        tree[node].sum += (end - start + 1) * tree[node].lazy;
        if (start != end) {
            tree[2 * node + 1].lazy += tree[node].lazy;
            tree[2 * node + 2].lazy += tree[node].lazy;
        }
        tree[node].lazy = 0;
    }

    if (start >= l && end <= r)
        return tree[node].sum;

    int mid = (start + end) / 2;
    int left_sum = queryRange(tree, 2 * node + 1, start, mid, l, r);
    int right_sum = queryRange(tree, 2 * node + 2, mid + 1, end, l, r);
    return left_sum + right_sum;
}

int main() {
    int *arr;
    int *result;
    SegmentTree* tree;
    int n, m;   

    printf("Enter the number of elements and the number of operations: ");
    if (scanf("%d %d", &n, &m) != 2) {
        fprintf(stderr, "Error reading the size of array and number of operations.\n");
        return 1; // Exit if the input format is incorrect
    }

    arr = (int *)malloc(n * sizeof(int));
    result = (int *)malloc(m * sizeof(int));
    tree = (SegmentTree*)malloc(4 * n * sizeof(SegmentTree));  // Segment tree array

    printf("Enter %d integers: ", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    int start = 0, end = 1<<((int)ceil(log2(n-1)));
    build(tree, arr, 0, start, end);
    int operation, l, r, tag;
    printf("Enter %d operations: ", m);
    for (int i = 0; i < m; i++) {
        scanf("%d", &operation);
        if (operation == 0) {
            scanf("%d%d%d", &l, &r, &tag);
            updateRange(tree, 0, start, end, l, r,tag==1);
            result[i] = 0;
        }
        else {
            scanf("%d%d", &l, &r);
            result[i] = queryRange(tree, 0, start, end, l, r);
            printf("%d\n", result[i]);
        }
    }
    free(tree);
    free(arr);
    for (int i = 0; i < m; i++) printf("%d, ", result[i]);
    free(result);
}



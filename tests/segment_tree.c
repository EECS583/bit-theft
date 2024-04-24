#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define MAX 1000  // Maximum array size (adjust as needed)

typedef struct {
    int64_t sum;
    int64_t lazy;
} SegmentTree;

void build(SegmentTree* tree, int64_t* arr, int64_t node, int64_t start, int64_t end) {
    if (start == end) {
        // Leaf node will have a single element
        tree[node].sum = arr[start];
        tree[node].lazy = 0;
    } else {
        int64_t mid = (start + end) / 2;
        build(tree, arr, 2 * node + 1, start, mid);
        build(tree, arr, 2 * node + 2, mid + 1, end);
        tree[node].sum = tree[2 * node + 1].sum + tree[2 * node + 2].sum;
        tree[node].lazy = 0;
    }
}

void updateRange(SegmentTree* tree, int64_t node, int64_t start, int64_t end, int64_t l, int64_t r, bool val) {
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
        int64_t delta = (val * 2 - 1);
        
        tree[node].sum += (end - start + 1) * delta;
        if (start != end) {
            tree[2 * node + 1].lazy += delta;
            tree[2 * node + 2].lazy += delta;
        }
        return;
    }

    int64_t mid = (start + end) / 2;
    updateRange(tree, 2 * node + 1, start, mid, l, r, val);
    updateRange(tree, 2 * node + 2, mid + 1, end, l, r, val);
    tree[node].sum = (tree[2 * node + 1].sum + tree[2 * node + 2].sum) % 100007;
}

int64_t queryRange(SegmentTree* tree, int64_t node, int64_t start, int64_t end, int64_t l, int64_t r) {
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
        return tree[node].sum % 100007;

    int64_t mid = (start + end) / 2;
    int64_t left_sum = queryRange(tree, 2 * node + 1, start, mid, l, r);
    int64_t right_sum = queryRange(tree, 2 * node + 2, mid + 1, end, l, r);
    return (left_sum + right_sum) % 100007;
}

int main() {
    int64_t *arr;
    int64_t *result;
    SegmentTree* tree;
    int64_t n, m;   

    srand(time(NULL));
    printf("Enter the number of elements and the number of operations: ");
    if (scanf("%lld %lld", &n, &m) != 2) {
        fprintf(stderr, "Error reading the size of array and number of operations.\n");
        return 1; // Exit if the input format is incorrect
    }

    arr = (int64_t *)malloc(n * sizeof(int64_t));
    result = (int64_t *)malloc(m * sizeof(int64_t));
    tree = (SegmentTree*)malloc(4 * n * sizeof(SegmentTree));  // Segment tree array

    // printf("Enter %lld integers: ", n);
    for (int64_t i = 0; i < n; i++) {
        // scanf("%lld", &arr[i]);
        arr[i] = rand() % 20;
    }

    int64_t start = 0, end = 1<<((int64_t)ceil(log2(n-1)));
    build(tree, arr, 0, start, end);
    int64_t operation, l, r, tag;
    printf("Enter %lld operations: ", m);
    result[0] = queryRange(tree, 0, start, end, start, end);
    for (int64_t i = 1; i < m; i++) {
        // scanf("%lld", &operation);
        operation = rand() % 2;
        l = rand() % n;
        r = rand() % (n-l) + l;
        // printf("%lld %lld %lld\n", operation, l, r);
        if (operation == 0) {
            // scanf("%lld%lld%lld", &l, &r, &tag);
            tag = rand() % 2;
            // printf("tag: %lld\n", tag);
            updateRange(tree, 0, start, end, l, r, tag==1);
            result[i] = 0;
        }
        else {
            // scanf("%lld%lld", &l, &r);
            result[i] = queryRange(tree, 0, start, end, l, r);
            // printf("%lld\n", result[i]);
        }
    }
    free(tree);
    free(arr);
    for (int i = 0; i < m; i++) printf("%llu, ", result[i]);
    free(result);
}



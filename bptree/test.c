#include "../common.c"
#define BPTREE_IMPLEMENTATION
#include "bptree.h"

// https://github.com/habedi/bptree
// commit: ac54a58, checked out on 2025-04-05

typedef struct aux_s {
    uint32_t key, cnt;
} aux_t;

// Fixed compare function: both parameters are properly cast now.
static inline int aux_cmp(const void *a, const void *b, const void *udata) {
    const aux_t *p = (const aux_t *)a;
    const aux_t *q = (const aux_t *)b;
    return ((p->key > q->key) - (p->key < q->key));
}

// Modified test_int: each aux_t is allocated on the heap (not on the stack as before).
// Note that bptree_free(h) does not free your user data,
// so we iterate over the tree using its iterator and free all items.
void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp) {
    uint32_t step = (N - n0) / (n_cp - 1);
    uint32_t i, n, j, cnt = 0;
    uint64_t z = 0, x = x0;

    // Create the B+tree. Passing NULL for allocators uses the default ones.
    bptree *h = bptree_new(256, aux_cmp, NULL, NULL, NULL, false);
    if (!h) {
        fprintf(stderr, "Failed to create bptree\n");
        exit(EXIT_FAILURE);
    }

    // Process items in checkpoints.
    for (j = 0, i = n0, n = n0; j < n_cp; ++j, n += step) {
        for (; i < n; ++i) {
            // Allocate a new aux_t on the heap.
            aux_t *a = malloc(sizeof(aux_t));
            if (!a) {
                fprintf(stderr, "Memory allocation error\n");
                exit(EXIT_FAILURE);
            }
            uint64_t y = udb_splitmix64(&x);
            a->key = udb_get_key(n, y);
            a->cnt = 1;

            // Try to locate an existing entry.
            aux_t *p = bptree_get(h, a);
            if (is_del) {
                if (p == NULL) {
                    // If not found, insert the new item.
                    bptree_put(h, a);
                    ++z;
                } else {
                    // If found, remove it and free the allocated memory.
                    bptree_remove(h, a);
                    free(a);
                }
            } else {
                if (p == NULL) {
                    // Insert the new item.
                    bptree_put(h, a);
                    ++z;
                    ++cnt;
                } else {
                    // If it exists, update its count and free the new allocation.
                    z += ++p->cnt;
                    free(a);
                }
            }
        }
        udb_measure(n, cnt, z, &cp[j]);
    }

    // Since bptree_free() does not free user data (aux_t items),
    // traverse the tree and free each allocated aux_t.
    bptree_iterator *iter = bptree_iterator_new(h);
    if (iter) {
        void *item;
        while ((item = bptree_iterator_next(iter)) != NULL) {
            free(item);
        }
        bptree_iterator_free(iter, free);
    }
    bptree_free(h);
}

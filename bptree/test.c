#include "../common.c"
#define BPTREE_IMPLEMENTATION
#include "bptree.h"

// https://github.com/habedi/bptree
// commit: ac54a58, checked out on 2025-04-05

typedef struct aux_s {
	uint32_t key, cnt;
} aux_t;

static inline int aux_cmp(const void *a, const void *b, const void *udata)
{
	const aux_t *p = (const aux_t*)a, *q = (const aux_t*)q;
	return ((p->key > q->key) - (p->key < q->key));
}

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j, cnt = 0;
	uint64_t z = 0, x = x0;
	bptree *h = bptree_new(256, aux_cmp, 0, 0, 0, false);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			aux_t a, *p;
			uint64_t y = udb_splitmix64(&x);
			a.key = udb_get_key(n, y), a.cnt = 1;
			p = bptree_get(h, &a);
			if (is_del) {
				if (p == 0) {
					bptree_put(h, &a);
					++z;
				} else bptree_remove(h, &a);
			} else {
				if (p == 0) {
					bptree_put(h, &a);
					++z;
					++cnt;
				} else z += ++p->cnt;
			}
		}
		udb_measure(n, cnt, z, &cp[j]);
	}
	bptree_free(h);
}

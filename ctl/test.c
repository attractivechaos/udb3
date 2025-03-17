#include "../common.c"
#include <string.h>

typedef struct {
	uint32_t key, val;
} aux;

static inline size_t aux_hash(aux *a) { return udb_hash_fn(a->key); }
static inline int aux_eq(aux *a, aux *b) { return a->key == b->key; }
static inline void aux_free(aux *a) {}
static inline aux aux_copy(aux *self) { return *self; }

#define T aux
#include "ctl/unordered_map.h"

// https://github.com/rurban/ctl
// 741bf5f, cloned on 2025-03-17

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	umap_aux h = umap_aux_init(aux_hash, aux_eq);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			int found;
			uint64_t y = udb_splitmix64(&x);
			aux a;
			umap_aux_it p;
			a.key = udb_get_key(n, y), a.val = 0;
			p = umap_aux_insert_found(&h, a, &found);
			if (is_del) {
				if (!found) p.ref->val = i, ++z;
				else umap_aux_erase(&h, a);
			} else {
				if (!found) p.ref->val = 0;
				z += ++p.ref->val;
			}
		}
		udb_measure(n, umap_aux_size(&h), z, &cp[j]);
	}
	umap_aux_free(&h);
}

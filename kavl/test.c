#include "../common.c"
#include "kavl.h"

typedef struct aux_s {
	uint32_t key, cnt;
	KAVL_HEAD(struct aux_s) head;
} aux_t;
#define aux_cmp(a, b) (((a)->key > (b)->key) - ((a)->key < (b)->key))
KAVL_INIT(32, aux_t, head, aux_cmp)

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	aux_t *root = 0, *p, *q;
	p = (aux_t*)malloc(sizeof(aux_t));
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			p->key = udb_get_key(n, y);
			q = kavl_insert(32, &root, p, 0);
			if (is_del) {
			} else {
				if (p == q) p = (aux_t*)malloc(sizeof(aux_t));
				z += ++q->cnt;
			}
		}
		udb_measure(n, kavl_size(head, root), z, &cp[j]);
	}
	kavl_free(aux_t, head, root, free);
}

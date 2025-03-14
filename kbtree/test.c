#include "../common.c"
#include "kbtree.h"

typedef struct aux_s {
	uint32_t key, cnt;
} aux_t;

#define aux_cmp(a, b) (((a).key > (b).key) - ((a).key < (b).key))
KBTREE_INIT(32, aux_t, aux_cmp)

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	kbtree_t(32) *h;
	h = kb_init(32, KB_DEFAULT_SIZE);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			aux_t a, *p;
			uint64_t y = udb_splitmix64(&x);
			a.key = udb_get_key(n, y), a.cnt = 1;
			if (is_del) {
			} else {
				p = kb_getp(32, h, &a);
				if (p == 0) {
					kb_putp(32, h, &a);
					++z;
				} else z += ++p->cnt;
			}
		}
		udb_measure(n, kb_size(h), z, &cp[j]);
	}
	kb_destroy(32, h);
}

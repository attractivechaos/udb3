#include "../common.c"

// https://github.com/troydhanson/uthash
// downloaded on 2023-12-16
#include "uthash.h"

typedef struct {
	uint32_t key;
	uint32_t cnt;
	UT_hash_handle hh;
} intcell_t;

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j, n_unique = 0;
	uint64_t z = 0, x = x0;
	intcell_t *h = 0, *r, *tmp;
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			HASH_FIND_INT(h, &key, r);
			if (is_del) {
				if (r == 0) {
					r = (intcell_t*)malloc(sizeof(intcell_t));
					r->key = key, r->cnt = i;
					HASH_ADD_INT(h, key, r);
					++n_unique;
					++z;
				} else {
					HASH_DEL(h, r);
					free(r);
					--n_unique;
				}
			} else {
				if (r == 0) {
					r = (intcell_t*)malloc(sizeof(intcell_t));
					r->key = key, r->cnt = 0;
					HASH_ADD_INT(h, key, r);
					++n_unique;
				}
				z += ++r->cnt;
			}
		}
		udb_measure(n, n_unique, z, &cp[j]);
	}
	HASH_ITER(hh, h, r, tmp) {
		HASH_DEL(h, r);
		free(r);
	}
}

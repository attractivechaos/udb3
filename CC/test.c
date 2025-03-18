#include "../common.c"
#include "cc.h"

#define CC_HASH uint32_t, { return udb_hash_fn(val); }
#include "cc.h"

// https://github.com/JacksonAllan/CC
// commit f8e27cd, cloned on 2025-03-17

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	map(uint32_t, uint32_t) h;
	init(&h);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			uint32_t sz = size(&h);
			uint32_t *p = get_or_insert(&h, key, 0);
			if (is_del) {
				if (size(&h) == sz)
					erase_itr(&h, p);
				else *p = i, ++z;
			} else {
				z += ++*p;
			}
		}
		udb_measure(n, size(&h), z, &cp[j]);
	}
	cleanup(&h);
}

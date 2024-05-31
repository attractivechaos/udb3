#include "../common-block.c"
#include "khashl.h"

#ifdef USE_CACHED
KHASHL_CMAP_INIT(KH_LOCAL, blockmap_t, blockmap, udb_block_t, uint32_t, udb_hash_fn, udb_block_eq)
#else
KHASHL_MAP_INIT(KH_LOCAL, blockmap_t, blockmap, udb_block_t, uint32_t, udb_hash_fn, udb_block_eq)
#endif

void test_block(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	blockmap_t *h = blockmap_init();
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			khint_t k;
			int absent;
			uint64_t y = udb_splitmix64(&x);
			udb_block_t d;
			udb_get_key(n, y, &d);
			k = blockmap_put(h, d, &absent);
			if (is_del) {
				if (absent) kh_val(h, k) = i, ++z;
				else blockmap_del(h, k);
			} else {
				if (absent) kh_val(h, k) = 0;
				z += ++kh_val(h, k);
			}
		}
		udb_measure(n, kh_size(h), z, &cp[j]);
	}
	blockmap_destroy(h);
}

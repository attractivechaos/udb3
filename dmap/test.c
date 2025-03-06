#include "../common.c"
#include "dmap.h"

// downloaded from https://github.com/jamesnolanverran/dmap
// commit c781b24, checked out on 2025-03-05

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j, cnt = 0;
	uint64_t z = 0, x = x0;
	uint32_t *h = 0;
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			if (is_del) { // FIXME: not working
				uint32_t *val = dmap_get(h, &key);
				if (val) {
					dmap_delete(h, &key);
					--cnt;
				} else {
					dmap_insert(h, &key, i);
					++cnt;
				}
			} else {
				uint32_t v = 1, *val = dmap_get(h, &key);
				if (val) v += *val;
				else ++cnt;
				dmap_insert(h, &key, v);
				z += v;
			}
		}
		udb_measure(n, cnt, z, &cp[j]);
	}
	dmap_free(h);
}

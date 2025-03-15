#include "../common.c"

static inline uint64_t stc_hash_fn(const uint32_t *x)
{
	return udb_hash_fn(*x);
}

#define i_type hmap_32, uint32_t, uint32_t
#define i_hash stc_hash_fn
#include "hmap.h"

// https://github.com/jamesnolanverran/dmap
// commit a784c92, cloned on 2025-03-16

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	hmap_32 h = {0};
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			hmap_32_result ret = hmap_32_insert(&h, key, 1);
			if (is_del) {
				if (ret.inserted) ret.ref->second = i, ++z;
				else hmap_32_erase_entry(&h, ret.ref);
			} else {
				if (!ret.inserted) ++ret.ref->second;
				z += ret.ref->second;
			}
		}
		udb_measure(n, hmap_32_size(&h), z, &cp[j]);
	}
	hmap_32_drop(&h);
}

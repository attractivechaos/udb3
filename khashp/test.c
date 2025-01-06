#include "../common.c"
#include "khashp.h"

static khint_t hash_fn32(const void *p, uint32_t key_len)
{
	uint32_t x = *(uint32_t*)p;
	x ^= x >> 16;
	x *= 0x85ebca6bU;
	x ^= x >> 13;
	x *= 0xc2b2ae35U;
	x ^= x >> 16;
	return x;
}

static int key_eq32(const void *p1, const void *p2, uint32_t key_len)
{
	return *(uint32_t*)p1 == *(uint32_t*)p2;
}

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	khashp_t *h = khp_init(4, 4, hash_fn32, key_eq32);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			khint_t k;
			int absent;
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			k = khp_put(h, &key, &absent);
			if (is_del) {
				if (absent) {
					khp_set_val(h, k, &i);
					++z;
				} else khp_del(h, k);
			} else {
				uint32_t v = 0;
				if (!absent) khp_get_val(h, k, &v);
				z += ++v;
				khp_set_val(h, k, &v);
			}
		}
		udb_measure(n, khp_size(h), z, &cp[j]);
	}
	khp_destroy(h);
}

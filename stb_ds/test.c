#include "../common.c"
#define STB_DS_IMPLEMENTATION
// version v0.67; cloned on 2024-05-05
#include "stb_ds.h"

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	struct { uint32_t key; uint32_t value; } *h = NULL;
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			int k;
			uint32_t key;
			uint64_t y = udb_splitmix64(&x);
			key = udb_get_key(n, y);
			k = hmgeti(h, key);
			if (is_del) {
				if (k < 0) {
					hmput(h, key, i);
					++z;
				} else {
					hmdel(h, key);
				}
			} else {
				if (k < 0) {
					hmput(h, key, 1);
					++z;
				} else {
					z += ++h[k].value;
				}
			}
		}
		udb_measure(n, hmlen(h), z, &cp[j]);
	}
	hmfree(h);
}

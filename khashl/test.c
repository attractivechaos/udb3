#include "../common.c"
#include "khashl.h"

KHASHL_MAP_INIT(, intmap_t, intmap, uint32_t, int, udb_hash_fn, kh_eq_generic)

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, x, n, j;
	uint64_t z = 0;
	intmap_t *h = intmap_init();
	for (j = 0, i = 0, n = n0, x = x0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			khint_t k;
			int absent;
			x = udb_hash32(x);
			k = intmap_put(h, udb_get_key(N, x), &absent);
			if (is_del) {
				if (absent) ++z;
				else intmap_del(h, k);
			} else {
				if (absent) kh_val(h, k) = 0;
				z += ++kh_val(h, k);
			}
		}
		udb_measure(n, kh_size(h), z, &cp[j]);
	}
	intmap_destroy(h);
}

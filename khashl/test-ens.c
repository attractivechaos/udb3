#include "../common.c"
#include "khashl.h"

#define KH_SUB_SHIFT 6
#define KH_SUB_N     (1<<KH_SUB_SHIFT)
#define KH_SUB_MASK  (KH_SUB_N - 1)

KHASHL_MAP_INIT(KH_LOCAL, intmap_t, intmap, uint32_t, uint32_t, udb_hash_fn, kh_eq_generic)

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, x, n, j, s;
	uint64_t z = 0;
	intmap_t *h[KH_SUB_N];
	for (s = 0; s < KH_SUB_N; ++s)
		h[s] = intmap_init();
	for (j = 0, i = 0, n = n0, x = x0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			intmap_t *g;
			khint_t k;
			int absent;
			x = udb_hash32(x);
			uint32_t key = udb_get_key(n, x);
			g = h[key & KH_SUB_MASK];
			k = intmap_put(g, key, &absent);
			if (is_del) {
				if (absent) kh_val(g, k) = i, ++z;
				else intmap_del(g, k);
			} else {
				if (absent) kh_val(g, k) = 0;
				z += ++kh_val(g, k);
			}
		}
		uint32_t size = 0;
		for (s = 0; s < KH_SUB_N; ++s)
			size += kh_size(h[s]);
		udb_measure(n, size, z, &cp[j]);
	}
	for (s = 0; s < KH_SUB_N; ++s)
		intmap_destroy(h[s]);
}

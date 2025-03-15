#include "../common.c"
#include "m-dict.h"

// https://github.com/P-p-H-d/mlib
// commit c55bb6b, cloned on 2025-03-16

static inline int oor_equal_p(uint32_t k, unsigned char n)
{
	return (k == (uint32_t)(-n-1));
}

static inline void oor_set(uint32_t *k, unsigned char n)
{
	*k = (uint32_t)(-n-1);
}

DICT_OA_DEF2(intmap, uint32_t, M_OPEXTEND(M_BASIC_OPLIST, HASH(udb_hash_fn), OOR_EQUAL(oor_equal_p), OOR_SET(API_2(oor_set))), uint32_t, M_BASIC_OPLIST)

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	intmap_t h;
	intmap_init(h);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			uint32_t *p = intmap_get(h, key);
			if (is_del) {
				if (p == 0) {
					intmap_set_at(h, key, i);
					++z;
				} else intmap_erase(h, key);
			} else {
				if (p == 0) {
					intmap_set_at(h, key, 1);
					++z;
				} else z += ++*p;
			}
		}
		udb_measure(n, intmap_size(h), z, &cp[j]);
	}
	intmap_clear(h);
}

#include "../common.c"

#define NAME intmap_t
#define KEY_TY uint32_t
#define VAL_TY uint32_t
#define HASH_FN udb_hash_fn
#define CMPR_FN vt_cmpr_integer
// version 2.0.0; cloned on 2024-05-06
#include "verstable.h"


void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	intmap_t h;
	intmap_t_init(&h);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			size_t ori_size = intmap_t_size(&h);
			if (is_del) {
				intmap_t_itr itr = intmap_t_get_or_insert(&h, key, i);
				if (intmap_t_size(&h) == ori_size)
					intmap_t_erase_itr(&h, itr);
				else ++z;
			} else {
				intmap_t_itr itr = intmap_t_get_or_insert(&h, key, 0);
				z += ++itr.data->val;
			}
		}
		udb_measure(n, intmap_t_size(&h), z, &cp[j]);
	}
	intmap_t_cleanup(&h);
}

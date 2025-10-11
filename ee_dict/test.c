#include "../common.c"
#include "ee_dict.h"

static uint64_t key_hash_fn(const u8 *p, size_t len)
{
	EE_UNUSED(len);
	return udb_hash_fn(*(uint32_t*)p);
}

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	DictConfig config = ee_dict_config_new(0, key_hash_fn, ee_eq_u32, ee_cpy_u32, ee_cpy_u32);
	Dict h = ee_dict_new_conf_m(128, uint32_t, uint32_t, config);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			if (is_del) {
				if (ee_dict_contains(&h, EE_RECAST_U8(key))) {
					ee_dict_remove(&h, EE_RECAST_U8(key));
				} else {
					ee_dict_set(&h, EE_RECAST_U8(key), EE_RECAST_U8(i));
					++z;
				}
			} else {
				if (ee_dict_contains(&h, EE_RECAST_U8(key))) {
					uint32_t *p = ee_dict_at(&h, EE_RECAST_U8(key));
					z += ++*p;
				} else {
					uint32_t t = 1;
					ee_dict_set(&h, EE_RECAST_U8(key), EE_RECAST_U8(t));
					++z;
				}
			}
		}
		udb_measure(n, ee_dict_count(&h), z, &cp[j]);
	}
	ee_dict_free(&h);
}

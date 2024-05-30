#include "../common.c"
#include <glib.h>

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	GHashTable *h;
	h = g_hash_table_new(NULL, NULL);
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			gpointer v, ori_key;
			int absent;
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			if (is_del) {
				if (g_hash_table_lookup_extended(h, GINT_TO_POINTER(key), &ori_key, &v)) {
					g_hash_table_remove(h, GINT_TO_POINTER(key));
				} else {
					g_hash_table_insert(h, GINT_TO_POINTER(key), GINT_TO_POINTER(i));
					++z;
				}
			} else {
				uint32_t cnt = 0;
				if (g_hash_table_lookup_extended(h, GINT_TO_POINTER(key), &ori_key, &v)) {
					cnt = GPOINTER_TO_INT(v) + 1;
				} else {
					cnt = 1;
				}
				g_hash_table_insert(h, GINT_TO_POINTER(key), GINT_TO_POINTER(cnt));
				z += cnt;
			}
		}
		udb_measure(n, g_hash_table_size(h), z, &cp[j]);
	}
	g_hash_table_destroy(h);
}

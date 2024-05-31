#include "../common-block.c"

// https://github.com/skarupke/flat_hash_map
// cloned on 2018-09-29, which remain the latest on 2023-12-12
#include "bytell_hash_map.hpp"

struct Hasher {
	inline size_t operator()(const udb_block_t &x) const {
		return udb_hash_fn(x);
	}
};

struct EqFunc {
	inline size_t operator()(const udb_block_t &a, const udb_block_t &b) const {
		return udb_block_eq(a, b);
	}
};

void test_block(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	ska::bytell_hash_map<udb_block_t, uint32_t, Hasher, EqFunc> h;
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			udb_block_t key;
			udb_get_key(n, y, &key);
			if (is_del) {
				auto p = h.insert(std::pair<udb_block_t, uint32_t>(key, i));
				if (p.second == false) h.erase(p.first);
				else ++z;
			} else {
				z += ++h[key];
			}
		}
		udb_measure(n, h.size(), z, &cp[j]);
	}
}

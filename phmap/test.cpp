#include "../common.c"
#include <functional>

// https://github.com/martinus/robin-hood-hashing
// cloned on 2023-12-15
#include "phmap.h"

struct Hash32 {
	inline size_t operator()(const uint32_t x) const {
		return udb_hash_fn(x);
	}
};

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	phmap::parallel_flat_hash_map<uint32_t, uint32_t, Hash32> h;
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, x, n, j;
	uint64_t z = 0;
	for (j = 0, i = 0, n = n0, x = x0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			x = udb_hash32(x);
			uint32_t key = udb_get_key(n, x);
			if (is_del) {
				auto p = h.try_emplace(udb_get_key(n, x), i);
				if (p.second == false) h.erase(p.first);
				else ++z;
			} else {
				z += ++h[key];
			}
		}
		udb_measure(n, h.size(), z, &cp[j]);
	}
}

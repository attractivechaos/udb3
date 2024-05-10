#include "../common.c"
#include <functional>

#include "robin_hood.h"

#define KH_SUB_SHIFT 6
#define KH_SUB_N     (1<<KH_SUB_SHIFT)
#define KH_SUB_MASK  (KH_SUB_N - 1)

struct Hash32 {
	//using is_avalanching = void;
	inline size_t operator()(const uint32_t x) const {
		return udb_hash_fn(x);
	}
};

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	robin_hood::unordered_flat_map<uint32_t, uint32_t, Hash32> h[KH_SUB_N];
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			uint32_t low = udb_hash_fn(key) & KH_SUB_MASK;
			if (is_del) {
				auto p = h[low].try_emplace(key, i);
				if (p.second == false) h[low].erase(p.first);
				else ++z;
			} else {
				z += ++h[low][key];
			}
		}
		uint32_t size = 0;
		for (uint32_t s = 0; s < KH_SUB_N; ++s)
			size += h[s].size();
		udb_measure(n, size, z, &cp[j]);
	}
}

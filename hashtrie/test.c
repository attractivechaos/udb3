#include "../common.c"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

// adapted from https://github.com/skeeto/scratch/blob/master/misc/concurrent-hash-trie.c

#define new(a, t, n)  (t *)alloc(a, sizeof(t), _Alignof(t), n, 1)
#define s(cstr)       (str){(byte *)cstr, sizeof(cstr)-1}

typedef int           bool;
typedef unsigned char byte;
typedef ptrdiff_t     size;
typedef uintptr_t     uptr;

typedef struct {
	byte *beg;
	byte *end;
} arena;

static byte *alloc(arena *a, size objsize, size align, size count, bool zero)
{
	size avail = a->end - a->beg;
	size pad = -(uptr)a->beg & (align - 1);
	assert(count < (avail - pad) / objsize);
	a->beg += pad;
	byte *r = a->beg;
	a->beg += objsize * count;
	return zero ? memset(r, 0, objsize*count) : r;
}

typedef struct map map;
struct map {
	map *child[4];
	uint32_t  key;
	uint32_t  value;
};

// Thread-safe, lock-free insert/search.
static uint32_t *upsert(map **m, uint32_t key, arena *a)
{
	for (uint32_t h = udb_hash_fn(key);; h <<= 2) {
		map *n = __atomic_load_n(m, __ATOMIC_ACQUIRE);
		if (!n) {
			if (!a) {
				return 0;
			}
			arena rollback = *a;
			map *new = new(a, map, 1);
			new->key = key;
			int pass = __ATOMIC_RELEASE;
			int fail = __ATOMIC_ACQUIRE;
			if (__atomic_compare_exchange_n(m, &n, new, 0, pass, fail)) {
				return &new->value;
			}
			*a = rollback;
		}
		if (n->key == key) return &n->value;
		m = n->child + (h >> 30);
	}
	return 0;
}

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j, cnt = 0;
	uint64_t z = 0, x = x0;
	uint64_t cap = N * sizeof(map) / 2;
	byte *heap = malloc(cap);
	arena perm = {0};
	perm.beg = heap;
	perm.end = heap + cap;
	map *m = 0;
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			if (is_del) {
			} else {
				uint32_t *p = upsert(&m, key, &perm);
				if (*p == 0) ++cnt;
				z += ++*p;
			}
		}
		udb_measure(n, cnt, z, &cp[j]);
	}
	free(heap);
}

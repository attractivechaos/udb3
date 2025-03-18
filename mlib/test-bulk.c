#include "../common.c"
#include "m-dict.h"

static inline int oor_equal_p(uint32_t k, unsigned char n)
{
	return (k == (uint32_t)(-n-1));
}

static inline void oor_set(uint32_t *k, unsigned char n)
{
	*k = (uint32_t)(-n-1);
}

DICT_OA_DEF2(intmap, uint32_t, M_OPEXTEND(M_BASIC_OPLIST, HASH(udb_hash_fn), OOR_EQUAL(oor_equal_p), OOR_SET(API_2(oor_set))), uint32_t, M_BASIC_OPLIST)

static inline void
update_sum(intmap_t h, uint32_t const key, uint32_t *oldval, uint32_t *newval)
{
  (void) h;
  (void) key;
  // Sum the two values and set then both to the result
  *newval += *oldval;
  *oldval = *newval;
}

static inline void
update_del(intmap_t h, uint32_t const key, uint32_t *oldval, uint32_t *newval)
{
  if (*oldval == 0xFFFFFFFF) {
    *oldval = *newval;
    *newval = 1;
  } else {
    intmap_erase(h, key);
    *newval = 0;
  }
}

#define BUF_N 1024

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
  uint32_t step = (N - n0) / (n_cp - 1);
  uint32_t i, n, j;
  uint64_t z = 0, x = x0;
  intmap_t h;
  uint32_t values[BUF_N];
  uint32_t keys[BUF_N];
  intmap_init(h);
  for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
    if (is_del) {
      while (i < n) {
        unsigned num = M_MIN(BUF_N, n-i);
        for(unsigned k = 0; k < num ; k++) {
          values[k] = i + k;
          uint64_t y = udb_splitmix64(&x);
          uint32_t key = udb_get_key(n, y);
          keys[k] = key;
        }
        intmap_bulk_update(h, num, values, keys, -1, update_del);
        for(unsigned k = 0; k < num ; k++) {
          z += values[k];
        }
        i += num;
      }
    } else {
      while (i < n) {
        unsigned num = M_MIN(BUF_N, n-i);
        for(unsigned k = 0; k < num ; k++) {
          values[k] = 1;
          uint64_t y = udb_splitmix64(&x);
          uint32_t key = udb_get_key(n, y);
          keys[k] = key;
        }
        intmap_bulk_update(h, num, values, keys, 0, update_sum);
        for(unsigned k = 0; k < num ; k++) {
          z += values[k];
        }
        i += num;
      }
    }
    udb_measure(n, intmap_size(h), z, &cp[j]);
  }
  intmap_clear(h);
}

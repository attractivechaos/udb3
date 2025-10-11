// implementing https://github.com/sanjirhabib/benchmark

#include <stdlib.h>
#include <stdio.h>
#include "khashl.h"

KHASHL_MAP_INIT(KH_LOCAL, strmap_t, strmap, kh_cstr_t, kh_cstr_t, kh_hash_str, kh_eq_str)

static inline int int2str(char *b, int c)
{
	char buf[16]; // an integer can't be longer than 16
	int i, j, l = 0;
	unsigned x = c >= 0? c : -c;
	do { buf[l++] = x%10 + '0'; x /= 10; } while (x > 0);
	if (c < 0) buf[l++] = '-';
	for (i = l - 1, j = 0; i >= 0; --i) b[j++] = buf[i];
	b[j] = 0;
	return l;
}

strmap_t *genmap(int n, int m)
{
	char key_buf[20], val_buf[20];
	int i, absent;
	strmap_t *h = strmap_init();
	for (i = 0; i < n; ++i) {
		khint_t k;
		memcpy(key_buf, "id.", 3);
		int2str(key_buf + 3, i % m);
		memcpy(val_buf, "val.", 4);
		int2str(val_buf + 4, i);
		k = strmap_put(h, key_buf, &absent);
		if (absent) {
			kh_key(h, k) = strdup(key_buf);
		} else {
			free((char*)kh_val(h, k));
		}
		kh_val(h, k) = strdup(val_buf);
	}
	return h;
}

int main(int argc, char *argv[])
{
	int n = 5000000, m = 500000;
	khint_t k;
	strmap_t *h = genmap(n, m);
	printf("%d\n", kh_size(h));
	k = strmap_get(h, "id.10000");
	printf("%s\n", kh_val(h, k));
	kh_foreach(h, k) {
		free((char*)kh_key(h, k));
		free((char*)kh_val(h, k));
	}
	strmap_destroy(h);
	return 0;
}

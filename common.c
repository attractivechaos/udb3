#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

/***********************************
 * Measuring CPU time and peak RSS *
 ***********************************/

typedef struct {
	uint32_t n_input, table_size;
	uint64_t checksum;
	double t, mem;
} udb_checkpoint_t;

static double udb_cputime(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
	return r.ru_utime.tv_sec + r.ru_stime.tv_sec + 1e-6 * (r.ru_utime.tv_usec + r.ru_stime.tv_usec);
}

static long udb_peakrss(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
#ifdef __linux__
	return r.ru_maxrss * 1024;
#else
	return r.ru_maxrss;
#endif
}

static void udb_measure(uint32_t n_input, uint32_t table_size, uint64_t checksum, udb_checkpoint_t *cp)
{
	cp->t = udb_cputime();
	cp->mem = udb_peakrss();
	cp->n_input = n_input;
	cp->table_size = table_size;
	cp->checksum = checksum;
}

/******************
 * Key generation *
 ******************/

static inline uint32_t udb_hash32(uint32_t key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}

static inline uint32_t udb_get_key(const uint32_t n, const uint32_t x)
{
	return udb_hash32(x % (n>>2));
}

/**********************************************
 * For testing key generation time (baseline) *
 **********************************************/

uint64_t udb_traverse_rng(uint32_t n, uint32_t x0)
{
	uint64_t sum = 0;
	uint32_t i, x;
	for (i = 0, x = x0; i < n; ++i) {
		x = udb_hash32(x);
		sum += udb_get_key(n, x);
	}
	return sum;
}

/*****************************************************
 * This is the hash function used by almost everyone *
 *****************************************************/

static inline uint32_t udb_hash_fn(uint32_t key)
{
	return key;
}

/*****************
 * Main function *
 *****************/

void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp);

int main(int argc, char *argv[])
{
	int c;
	double t0, t_keygen;
	uint64_t sum;
	uint32_t i, n_cp = 11, N = 70000000, n0 = 10000000, x0 = 1, is_del = 0;
	udb_checkpoint_t cp0, *cp;

	while ((c = getopt(argc, argv, "n:N:0:k:d")) >= 0) {
		if (c == 'n') n0 = atol(optarg);
		else if (c == 'N') N = atol(optarg);
		else if (c == '0') x0 = atol(optarg);
		else if (c == 'k') n_cp = atoi(optarg);
		else if (c == 'd') is_del = 1;
	}

	printf("CL\tUsage: run-test [options]\n");
	printf("CL\tOptions:\n");
	printf("CL\t  -d         evaluate insertion/deletion (insertion only by default)\n");
	printf("CL\t  -N INT     total number of input items [%d]\n", N);
	printf("CL\t  -n INT     initial number of input items [%d]\n", n0);
	printf("CL\t  -k INT     number of checkpoints [%d]\n", n_cp);
	printf("CL\n");

	cp = (udb_checkpoint_t*)calloc(n_cp, sizeof(*cp));

	t0 = udb_cputime();
	sum = udb_traverse_rng(N, x0);
	t_keygen = udb_cputime() - t0;
	printf("TG\t%.3f\t%ld\n", t_keygen, (long)sum); // need to print sum; otherwise the compiler may optimize udb_traverse_rng() out

	udb_measure(0, 0, 0, &cp0);
	test_int(N, n0, is_del, x0, n_cp, cp);

	for (i = 0; i < n_cp; ++i) {
		double t, m;
		t = (cp[i].t - cp0.t - t_keygen * cp[i].n_input / N) / cp[i].n_input * 1e6;
		m = (cp[i].mem - cp0.mem) / cp[i].table_size;
		printf("M%c\t%d\t%d\t%lx\t%.3f\t%.3f\t%.4f\t%.2f\n", is_del? 'D' : 'I', cp[i].n_input, cp[i].table_size, (long)cp[i].checksum,
			cp[i].t - cp0.t, (cp[i].mem - cp0.mem) * 1e-6, t, m);
	}
	free(cp);
	return 0;
}

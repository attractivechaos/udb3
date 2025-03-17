#include "../common.c"
#include <Python.h>

// FIXME: this doesn't change the values
void test_int(uint32_t N, uint32_t n0, int32_t is_del, uint32_t x0, uint32_t n_cp, udb_checkpoint_t *cp)
{
	uint32_t step = (N - n0) / (n_cp - 1);
	uint32_t i, n, j;
	uint64_t z = 0, x = x0;
	Py_Initialize();
	PyObject *h = PyDict_New();
	for (j = 0, i = 0, n = n0; j < n_cp; ++j, n += step) {
		for (; i < n; ++i) {
			uint64_t y = udb_splitmix64(&x);
			uint32_t key = udb_get_key(n, y);
			PyObject *k = PyLong_FromLong(key);
			if (is_del) {
				if (!PyDict_Contains(h, k)) {
					PyObject *v = PyLong_FromLong(1);
					PyDict_SetItem(h, k, v);
					Py_DECREF(v);
				} else {
					PyDict_DelItem(h, k);
				}
			} else {
				if (!PyDict_Contains(h, k)) {
					PyObject *v = PyLong_FromLong(1);
					PyDict_SetItem(h, k, v);
					Py_DECREF(v);
				}
			}
			Py_DECREF(k);
		}
		udb_measure(n, PyDict_Size(h), z, &cp[j]);
	}
	Py_DECREF(h);
	Py_Finalize();
}

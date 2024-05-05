all:run-test

run-test:test.c ../common.c stb_ds.h
	$(CC) -O3 -Wall $< -o $@

clean:
	rm -fr run-test

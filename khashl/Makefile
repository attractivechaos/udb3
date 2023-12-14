all:run-test run-test-ens

run-test:test.c ../common.c khashl.h
	$(CC) -O3 -Wall $< -o $@

run-test-ens:test-ens.c ../common.c khashl.h
	$(CC) -O3 -Wall $< -o $@

clean:
	rm -fr run-test run-test-ens

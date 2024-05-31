EXE=run-test run-test-ens run-test-blk-raw run-test-blk-cached

all:$(EXE)

run-test:test.c ../common.c khashl.h
	$(CC) -O3 -Wall $< -o $@

run-test-ens:test-ens.c ../common.c khashl.h
	$(CC) -O3 -Wall $< -o $@

run-test-blk-raw:test-block.c ../common-block.c khashl.h
	$(CC) -O3 -Wall $< -o $@

run-test-blk-cached:test-block.c ../common-block.c khashl.h
	$(CC) -O3 -DUSE_CACHED -Wall $< -o $@

clean:
	rm -fr $(EXE)

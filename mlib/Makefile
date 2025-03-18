all:run-test run-test.bulk

run-test:test.c ../common.c m-dict.h
	$(CC) -O3 -Wall -DNDEBUG $< -o $@

run-test.bulk:test-bulk.c ../common.c m-dict.h
	$(CC) -O3 -Wall -DNDEBUG $< -o $@

clean:
	rm -fr run-test run-test.bulk

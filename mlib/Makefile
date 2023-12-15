all:run-test

run-test:test.c ../common.c m-dict.h
	$(CC) -O3 -Wall -DNDEBUG $< -o $@

clean:
	rm -fr run-test

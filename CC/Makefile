all:run-test

run-test:test.c ../common.c cc.h
	$(CC) -O3 -Wall $< -o $@

clean:
	rm -fr run-test

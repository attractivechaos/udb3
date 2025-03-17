all:run-test

run-test:test.c ../common.c
	$(CC) -O3 -Wall -I. $< -o $@

clean:
	rm -fr run-test

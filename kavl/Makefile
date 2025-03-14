EXE=run-test

all:$(EXE)

run-test:test.c ../common.c kavl.h kmempool.h kmempool.c
	$(CC) -O3 -Wall $< kmempool.c -o $@

clean:
	rm -fr $(EXE)

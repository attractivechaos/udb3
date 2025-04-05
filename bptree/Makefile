EXE=run-test

all:$(EXE)

run-test:test.c ../common.c bptree.h
	$(CC) -O3 -Wall $< -o $@

clean:
	rm -fr $(EXE)

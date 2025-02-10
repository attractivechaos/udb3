EXE=run-test

all:$(EXE)

run-test:test.c ../common.c dmap.c dmap.h
	$(CC) -O3 -Wall $< dmap.c -o $@

clean:
	rm -fr $(EXE)

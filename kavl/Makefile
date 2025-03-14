EXE=run-test

all:$(EXE)

run-test:test.c ../common.c kavl.h
	$(CC) -O3 -Wall $< -o $@

clean:
	rm -fr $(EXE)

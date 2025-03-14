EXE=run-test

all:$(EXE)

run-test:test.c ../common.c
	$(CC) -O3 -Wall $< -o $@

clean:
	rm -fr $(EXE)

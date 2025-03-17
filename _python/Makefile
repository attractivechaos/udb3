EXE=run-test

all:$(EXE)

run-test:test.c ../common.c
	$(CC) -O3 -Wall `python3.12-config --includes` `python3.12-config --libs` $< -o $@

clean:
	rm -fr $(EXE)

EXE=run-test

all:$(EXE)

run-test:test.c ../common.c ee_core.h ee_dict.h
	$(CC) -std=gnu99 -O3 -Wall -mavx2 $< -o $@

clean:
	rm -fr $(EXE)

all:run-test

run-test:test.c
	gcc -O2 $< -o $@ `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`

clean:
	rm -f run-test

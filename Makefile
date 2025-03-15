all:
	for dir in `ls | egrep -v '^(Makefile|common.c|common_block.c|emilib2|_)'`; do (cd $$dir && make); done

clean:
	rm -f */run-test

all:run-test run-test-blk

run-test:test.cpp ../common.c
	$(CXX) -O3 -Wall -std=c++14 $< -o $@

run-test-blk:test-block.cpp ../common-block.c
	$(CXX) -O3 -Wall -std=c++14 $< -o $@

clean:
	rm -f run-test run-test-blk

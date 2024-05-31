all:run-test run-test-ens run-test-blk

run-test:test.cpp ../common.c robin_hood.h
	$(CXX) -O3 -Wall -std=c++11 $< -o $@

run-test-ens:test-ens.cpp ../common.c robin_hood.h
	$(CXX) -O3 -Wall -std=c++11 $< -o $@

run-test-blk:test-block.cpp ../common-block.c robin_hood.h
	$(CXX) -O3 -Wall -std=c++11 $< -o $@

clean:
	rm -f run-test run-test-ens run-test-blk

BOOST_ROOT=.

all:run-test run-test-ens run-test-blk

run-test:test.cpp ../common.c
	$(CXX) -O3 -Wall -std=c++11 -I$(BOOST_ROOT) $< -o $@

run-test-ens:test-ens.cpp ../common.c
	$(CXX) -O3 -Wall -std=c++11 -I$(BOOST_ROOT) $< -o $@

run-test-blk:test-block.cpp ../common-block.c
	$(CXX) -O3 -Wall -std=c++11 -I$(BOOST_ROOT) $< -o $@

clean:
	rm -f run-test run-test-ens run-test-blk

all:run-test-ens

run-test:test.cpp ../common.c robin_hood.h
	$(CXX) -O3 -Wall -std=c++11 $< -o $@

run-test-ens:test-ens.cpp ../common.c robin_hood.h
	$(CXX) -O3 -Wall -std=c++11 $< -o $@

clean:
	rm -f run-test

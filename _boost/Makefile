BOOST_ROOT=.

all:run-test

run-test:test.cpp ../common.c
	$(CXX) -O3 -Wall -std=c++11 -I$(BOOST_ROOT) $< -o $@

clean:
	rm -f run-test

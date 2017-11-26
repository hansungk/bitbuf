CXXFLAGS += -std=c++11 -g -Wall

all: test

test: bitbuf_test.cpp bitbuf.o
	$(CXX) $(CXXFLAGS) -o bitbuf-test $^
	./bitbuf-test

bitbuf_test.o: bitbuf_test.cpp bitbuf.o
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o bitbuf-test

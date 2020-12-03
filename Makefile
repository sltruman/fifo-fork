CXX = g++
CXXFLAGS = -std=c++14
LDFLAGS = -lpthread

fifo-fork: main.cpp Makefile
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

.PHONY : clean install uninstall test

test: fifo-fork
	./$< faded.264 -o 1.264 2.264 3.264 

clean: fifo-fork
	rm $^
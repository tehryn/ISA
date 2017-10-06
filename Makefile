all: popser

popser: popser.o process_requests.o
	g++ -std=c++11 -g popser.o process_requests.o -o popser -lpthread

popser.o: popser.cpp popser.hpp
	g++ -std=c++11 -c -g popser.cpp

process_requests.o: process_requests.cpp process_requests.hpp
	g++ -std=c++11 -c -g process_requests.cpp

.PHONY: clean
clean:
	rm popser.o process_requests.o Mail_dir.o

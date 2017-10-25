all: popser

CLASS=Mail_dir.hpp Mail_file.hpp
DEBUG=-DDEBUG

popser: popser.o process_requests.o md5.o
	g++ -std=c++11 -g popser.o process_requests.o md5.o -o popser -lpthread $(DEBUG)

popser.o: popser.cpp popser.hpp $(CLASS)
	g++ -std=c++11 -c -g popser.cpp $(DEBUG)

process_requests.o: process_requests.cpp process_requests.hpp $(CLASS)
	g++ -std=c++11 -c -g process_requests.cpp $(DEBUG)

md5.o: md5.hpp
	g++ -std=c++11 -c -g md5.cpp

.PHONY: clean
clean:
	rm popser.o process_requests.o md5.o popser	

all: popser

CLASSES=Mail_dir.hpp Mail_file.hpp
DEBUG=-DDEBUG

popser: popser.o process_requests.o md5.o globals.o
	g++ -std=c++11 -g popser.o process_requests.o md5.o globals.o -o popser -lpthread

popser.o: popser.cpp popser.hpp $(CLASSES)
	g++ -std=c++11 -c -g popser.cpp $(DEBUG)

process_requests.o: process_requests.cpp process_requests.hpp $(CLASSES)
	g++ -std=c++11 -c -g process_requests.cpp $(DEBUG)

md5.o: md5.hpp md5.cpp
	g++ -std=c++11 -c -g md5.cpp

globals.o: globals.hpp globals.cpp
	g++ -std=c++11 -c -g globals.cpp $(DEBUG)

.PHONY: clean
clean:
	rm -f popser.o process_requests.o md5.o globals.o popser

all: popser

popser: popser.o
	g++ -std=c++11 -g popser.o -o popser -lpthread

popser.o: popser.cpp Makefile
	g++ -std=c++11 -c -g popser.cpp

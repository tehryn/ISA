all: popser

popser: popser.o
	g++ -std=c++11 popser.o -o popser

popser.o: popser.cpp
	g++ -std=c++11 -c popser.cpp

all: 
	g++ -ggdb index.cpp -o index

check: all 
	./index

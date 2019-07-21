all: 
	g++ -ggdb ut_index.cpp -o index

check: all 
	./index

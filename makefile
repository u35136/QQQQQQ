all:
#	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o 2048 2048.cpp
	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o sol Threes_solve.cpp
clean:
	rm 2048

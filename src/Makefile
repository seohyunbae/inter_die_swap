all: linux


clean:
	rm ./main

linux:
	g++ -O3 -I /usr/include/eigen3 -lpthread -fopenmp ./DB/*.cpp ./FM/*.cpp ./GP/*.cpp ./LG/*.cpp ./main.cpp -o main

mac:
	g++ -O3 -I /opt/homebrew/include/eigen3 -fopenmp ./DB/*.cpp ./FM/*.cpp ./GP/*.cpp ./main.cpp -o main

centos:
	g++ -O3 -I /usr/include/eigen3 -lpthread -fopenmp -std=gnu++11 ./DB/*.cpp ./FM/*.cpp ./GP/*.cpp ./LG/*.cpp ./main.cpp -o main


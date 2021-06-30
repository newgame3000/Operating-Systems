FLAGS = -g -O2 -pthread -std=c++17

all: main.cpp computing_node.cpp
	g++ $(FLAGS) main.cpp -o solution -lzmqpp -lsodium -lzmq
	g++ $(FLAGS) computing_node.cpp -o cn -lzmqpp -lsodium -lzmq
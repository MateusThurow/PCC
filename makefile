all:
	g++ main.cpp lib/*.cpp -std=c++11 -o programa -ltbb -fopenmp -fcilkplus -pthread

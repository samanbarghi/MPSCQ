all:
	g++ -std=c++1y -g -o test test.cpp -pthread
	g++ -std=c++1y -g -o testBlocking testBlocking.cpp -pthread
clean:
	rm test testBlocking
all: cache
cache: cache.cpp
	g++ cache.cpp -o cache
clean:
	rm ./cache
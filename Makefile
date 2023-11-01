CC := g++

ifeq ($(OS),Windows_NT)
CFLAGS := -lws2_32 -std=c++11
else
CFLAGS := -std=c++11 -pthread
endif

all: client server proxy

client: client.cpp src/*.cpp
	$(CC) $^ -o $@ $(CFLAGS)

server: server.cpp src/*.cpp
	$(CC) $^ -o $@ $(CFLAGS)

proxy: proxy.cpp src/*.cpp
	$(CC) $^ -o $@ $(CFLAGS)

clean:
ifeq ($(OS),Windows_NT)
	del client.exe
	del server.exe
	del proxy.exe
else
	rm -f client
	rm -f server
	rm -f proxy
endif


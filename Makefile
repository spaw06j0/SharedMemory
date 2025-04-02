# Makefile for Linux shared memory example

# Compiler options
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -lrt -pthread

# OpenCV flags (for the client)
CLIENT_CXXFLAGS = $(CXXFLAGS) -I/usr/include/opencv4 $(shell pkg-config --cflags opencv4)
CLIENT_LDFLAGS = $(LDFLAGS) $(shell pkg-config --libs opencv4)

# Source files
SERVER_SRCS = test/testserver.cpp sharedmem.cpp
CLIENT_SRCS = test/testclient.cpp sharedmem.cpp

# Object files
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)

# Targets
all: server client

server: $(SERVER_OBJS)
	$(CXX) $(SERVER_OBJS) -o ai_server $(LDFLAGS)

client: $(CLIENT_OBJS)
	$(CXX) $(CLIENT_OBJS) -o ai_client $(CLIENT_LDFLAGS)

# Server object files
test/testserver.o: test/testserver.cpp sharedmem.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Client object files
test/testclient.o: test/testclient.cpp sharedmem.h
	$(CXX) $(CLIENT_CXXFLAGS) -c $< -o $@

# Common object files
sharedmem.o: sharedmem.cpp sharedmem.h
	$(CXX) $(CLIENT_CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) ai_server ai_client

.PHONY: all clean

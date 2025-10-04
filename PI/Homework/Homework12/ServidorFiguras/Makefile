# Linux Makefile
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c -std=c++11
LDFLAGS := 
DEPS := Socket.h FileSystem.h
TARGETS := ForkMirrorServer.out MirrorClient.out ThreadMirrorServer.out

ALL := $(TARGETS)
$(info $$TARGETS = $(TARGETS))

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.out: %.o VSocket.o Socket.o FileSystem.o
	$(info $$^ = $^)
	$(CXX) $^ $(LDFLAGS) -o $@

ForkMirrorServer.out:	ForkMirrorServer.o VSocket.o Socket.o FileSystem.o
	$(CXX) ForkMirrorServer.o VSocket.o Socket.o FileSystem.o $(LDFLAGS) -o $@
	
ThreadMirrorServer.out:	ThreadMirrorServer.o VSocket.o Socket.o 
	$(CXX) ThreadMirrorServer.o VSocket.o Socket.o $(LDFLAGS) -o $@

MirrorClient.out:	MirrorClient.o VSocket.o Socket.o 
	$(CXX) MirrorClient.o VSocket.o Socket.o $(LDFLAGS) -o $@

FileSystem.o: FileSystem.cpp FileSystem.h
	$(CXX) $(CXXFLAGS) FileSystem.cpp

clean:
	rm -f *.o *.out disco.bin
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g
LDFLAGS = -lssl -lcrypto -pthread

# fuentes
SRCS = Socket.cc SSLSocket.cc VSocket.cc
OBJS = $(SRCS:.cc=.o)

# ejecutables
CLIENT = SSLClient
SERVER = SSLServer

all: $(CLIENT) $(SERVER)

$(CLIENT): SSLClient.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(SERVER): SSLServer.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(CLIENT) $(SERVER)

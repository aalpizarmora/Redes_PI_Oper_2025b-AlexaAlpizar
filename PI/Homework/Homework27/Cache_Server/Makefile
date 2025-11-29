# ==== Config ====
CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -g
INCLUDES  := -I./Cache_Manager -I./Server
LDLIBS    := -lssl -lcrypto -lpthread

BIN_DIR   := bin
SERVER_BIN:= $(BIN_DIR)/cache_server
CLIENT_BIN:= $(BIN_DIR)/cache_client

# ==== Fuentes comunes (sin mains) ====
COMMON_SRCS := \
  Cache_Manager/CacheManager.cpp \
  Cache_Manager/VMM.cpp \
  Server/SSLSocket.cc \
  Server/Socket.cc \
  Server/VSocket.cc

COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)
COMMON_OBJS := $(COMMON_OBJS:.cc=.o)

# ==== Mains ====
SERVER_MAIN := Server/SSLServer.cc
CLIENT_MAIN := Server/SSLClient.cpp

SERVER_OBJ  := $(SERVER_MAIN:.cc=.o)
CLIENT_OBJ  := $(CLIENT_MAIN:.cpp=.o)

# ==== Targets por defecto ====
all: $(SERVER_BIN) $(CLIENT_BIN)

# ---- Servidor ----
$(SERVER_BIN): $(COMMON_OBJS) $(SERVER_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDLIBS)

# ---- Cliente ----
$(CLIENT_BIN): $(COMMON_OBJS) $(CLIENT_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDLIBS)

# ==== Compilación genérica ====
Cache_Manager/%.o: Cache_Manager/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

Server/%.o: Server/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

Server/%.o: Server/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ==== Atajos ====
run-server: $(SERVER_BIN)
	./$(SERVER_BIN) $(filter-out $@,$(MAKECMDGOALS))

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN) $(filter-out $@,$(MAKECMDGOALS))

clean:
	rm -f Cache_Manager/*.o Server/*.o
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

help:
	@echo "Targets:"
	@echo "  make                -> compila server y client"
	@echo "  make run-server 8082 -> ejecuta el server en puerto 6752"
	@echo "  make run-client host port -> ejecuta el cliente (por defecto localhost 6752)"
	@echo "  make clean          -> borra objetos y binarios"

.PHONY: all run-server run-client clean help

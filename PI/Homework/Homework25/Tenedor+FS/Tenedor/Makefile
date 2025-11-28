OPENSSL_PREFIX := $(shell brew --prefix openssl 2>/dev/null)

ifneq ($(OPENSSL_PREFIX),)
    CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I$(OPENSSL_PREFIX)/include
    LDFLAGS = -L$(OPENSSL_PREFIX)/lib -lssl -lcrypto -pthread
else
    # Fallback a rutas por defecto
    CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
    LDFLAGS = -lssl -lcrypto -pthread
endif

CXX = g++

# Archivos comunes
COMMON_SRC = Socket.cc VSocket.cc SSLSocket.cc FileSystem.cpp
COMMON_OBJ = $(COMMON_SRC:.cc=.o)
COMMON_OBJ := $(COMMON_OBJ:.cpp=.o)

# Ejecutables
EXEC_DRAW = DrawServer
EXEC_TENEDOR = Tenedor

# Programas finales
all: $(EXEC_DRAW) $(EXEC_TENEDOR)

# --- DrawServer ---
$(EXEC_DRAW): DrawServer.o $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

DrawServer.o: DrawServer.cpp FileSystem.h Socket.h VSocket.h SSLSocket.h

# --- Tenedor ---
$(EXEC_TENEDOR): tenedorServer.o $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

tenedorServer.o: tenedorServer.cpp Socket.h VSocket.h SSLSocket.h

# --- Compilación genérica de .cpp y .cc ---
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Limpieza ---
clean:
	rm -f *.o *.d $(EXEC_DRAW) $(EXEC_TENEDOR)

.PHONY: all clean
# Linux/MacOS Makefile
# Compilación automática de todos los .cc
#

CXX      := g++
# Use the correct OpenSSL path - try both common locations
OPENSSL_INCLUDE := $(shell brew --prefix openssl)/include
CXXFLAGS := -g -c -Wall -I$(OPENSSL_INCLUDE)
LDFLAGS  := -L$(shell brew --prefix openssl)/lib -lssl -lcrypto

# Detecta todos los archivos .cc en la carpeta
SRCS := $(wildcard *.cc)

# Genera los objetos correspondientes
OBJS := $(SRCS:.cc=.o)

# Ejecutable final
TARGET := main.out

# Regla por defecto
all: $(TARGET)

# Compilar cada archivo .cc a .o
%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

# Enlazar todos los objetos en el ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# Limpiar archivos generados
clean:
	rm -f *.o *.out
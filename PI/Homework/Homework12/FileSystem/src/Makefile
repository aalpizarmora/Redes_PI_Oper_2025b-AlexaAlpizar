# Nombre del compilador
CXX = g++

# Opciones de compilación
CXXFLAGS = -Wall -Wextra -std=c++17

# Archivos fuente
SRC = main.cpp FileSystem.cpp

# Archivos objeto (se generan a partir de los .cpp)
OBJ = $(SRC:.cpp=.o)

# Nombre del ejecutable final
TARGET = programa

# Regla por defecto
all: $(TARGET)

# Regla para crear el ejecutable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regla para compilar cada archivo fuente a objeto
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar los archivos generados
clean:
	rm -f $(OBJ) $(TARGET)
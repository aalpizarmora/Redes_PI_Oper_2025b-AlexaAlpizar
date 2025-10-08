# =========================
# Makefile simple y limpio
# =========================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread -g \
            -I. -I./core -I./core/FileSystem -I./common

# Carpetas
APP_DIR     := ./app
CORE_DIR    := ./core
COMMON_DIR  := ./common
BIN_DIR     := ./bin

# (Opcional) archivo de unidad si quieres borrarlo en 'clean'
UNIT_FILE   := ./unidad.bin

# Archivos fuente (RUTAS ACTUALIZADAS)
SRCS := $(CORE_DIR)/FileSystem/FileSystem.cpp \
        $(CORE_DIR)/servidor.cpp \
        $(CORE_DIR)/tenedor.cpp \
        $(COMMON_DIR)/Client.cpp \
        $(COMMON_DIR)/Thread.cpp \
        $(COMMON_DIR)/Semaphore.cpp \
        $(APP_DIR)/main.cpp

# Objetos generados (respeta subcarpetas)
OBJS := $(SRCS:.cpp=.o)

# Ejecutable final
TARGET := $(BIN_DIR)/fs_app

.PHONY: all clean distclean run reset-disk

# =========================
# Regla principal
# =========================
all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# =========================
# Regla para objetos
# =========================
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================
# Limpieza
# =========================
clean:
	rm -f $(OBJS)
	rm -rf $(BIN_DIR)
	rm -f $(UNIT_FILE)

distclean: clean
	@true

reset-disk:
	rm -f $(UNIT_FILE)

# =========================
# Ejecución rápida
# =========================
run: $(TARGET)
	$(TARGET)

# =========================
# Makefile simple y limpio
# =========================

CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -pthread -g -MMD -MP
INCLUDES  := -I. -I./core -I./core/FileSystem -I./core/CacheServer -I./common
LDFLAGS   :=
LDLIBS    :=

# Carpetas
APP_DIR     := ./app
CORE_DIR    := ./core
COMMON_DIR  := ./common
BIN_DIR     := ./bin

# (Opcional) archivo de unidad si quieres borrarlo en 'clean'
UNIT_FILE   := ./unidad.bin

# ============
# Fuentes
# ============
SRCS := \
  $(wildcard $(CORE_DIR)/FileSystem/*.cpp) \
  $(wildcard $(CORE_DIR)/CacheServer/*.cpp) \
  $(CORE_DIR)/servidor.cpp \
  $(CORE_DIR)/tenedor.cpp \
  $(wildcard $(COMMON_DIR)/*.cpp) \
  $(APP_DIR)/main.cpp

# Objetos y dependencias (misma jerarquía de carpetas)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

# Ejecutable final
TARGET := $(BIN_DIR)/fs_app

.PHONY: all clean distclean run reset-disk tree

# =========================
# Regla principal
# =========================
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# =========================
# Reglas de compilación
# =========================
# Compila cualquier .cpp a .o, respetando subcarpetas y generando .d
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Incluye dependencias si existen (no falla si aún no están)
-include $(DEPS)

# =========================
# Limpieza
# =========================
clean:
	@rm -f $(OBJS) $(DEPS)
	@rm -rf $(BIN_DIR)
	@rm -f $(UNIT_FILE)

distclean: clean
	@true

reset-disk:
	@rm -f $(UNIT_FILE)

# =========================
# Ejecución rápida
# =========================
run: $(TARGET)
	$(TARGET)

# (Opcional) ver árbol de fuentes que detecta
tree:
	@echo "SRCS:"; printf "  %s\n" $(SRCS)
	@echo "OBJS:"; printf "  %s\n" $(OBJS)

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include "VMM.h"

class CacheManager {
public:
  CacheManager();

  // true si existe al menos un bloque de la figura y tiene datos
  bool contains(const std::string& figureName);

  // Almacena la figura en bloques de 256B. Retorna false si hay error
  bool store(const std::string& figureName, const std::string& content);

  // Reconstruye la figura concatenando sus bloques por chunkIndex.
  std::string retrieve(const std::string& figureName);

  // Borrado logico: deja chunkSize=0 en sus bloques y quita a la figura del indice.
  bool invalidate(const std::string& figureName);

  // Lista de claves actualmente presentes según índice (filtradas por contains()).
  std::string listKeys();

private:
  VirtualMemoryManager vmm;

   // "cat"-> [5, 6], la figura "cat" ocupa los bloques 5 y 6
  std::unordered_map<std::string, std::vector<int>> indexTable;

  // Auxiliares
  static bool isValidName(const std::string& name);
  void pruneStaleBlocks(const std::string& figureName); // limpia bloques expulsados
  void ensureFigureIndex(const std::string& figureName); // crea entrada vacia si no existe
};

#include "CacheManager.h"
#include <cstring> 
#include <sstream>
#include <iostream>

CacheManager::CacheManager() : vmm() {}

// Validamos que el nombre no exceda los 9 caracteres permitidos.
bool CacheManager::isValidName(const std::string& name) {
  return !name.empty() && name.size() <= 9;
}


// Asegura que exista una entrada en el indice para la figura.
void CacheManager::ensureFigureIndex(const std::string& figureName) {
  if (!indexTable.count(figureName)) {
    indexTable[figureName] = {};
  }
}

// Depura el indice de la figura eliminando los numeros de bloque que ya no tienen datos validos por si FIFO ya los expulso
void CacheManager::pruneStaleBlocks(const std::string& figureName) {
  // Si no existe en el indice, no hacemos nada
  if (!indexTable.count(figureName)) {
    return;
  }

  std::vector<int>& blocks = indexTable[figureName];
  std::vector<int> nuevos;

  // Revisamos cada bloque y guardamos solo los que siguen teniendo datos
  for (size_t i = 0; i < blocks.size(); i++) {
    int b = blocks[i];
    uint8_t chunk = 0;
    char buffer[CACHE_BLOCK_SIZE];
    uint16_t n = vmm.readBlock(b, buffer, CACHE_BLOCK_SIZE, &chunk);

    if (n > 0) {
      nuevos.push_back(b);
    }
  }

  // Reemplazamos la lista por solo los bloques validos
  blocks = nuevos;

  // Si ya no quedan bloques, quitamos la figura del indice
  if (blocks.empty()) {
    indexTable.erase(figureName);
  }
}

bool CacheManager::contains(const std::string& figureName) {
  //si el nombre no es valido, retorna false
  if (!isValidName(figureName)) {
    std::cerr << "Error: nombre de figura inválido: '" << figureName << "'\n";
    return false;
  }

  // Usamos findBlockNumber() para localizar un bloque de esa figura
  char nameBuf[10] = {0};
  std::strncpy(nameBuf, figureName.c_str(), 9);

  int8_t block = vmm.findBlockNumber(nameBuf);
  if (block < 0) {
    std::cerr << "Error: figura '" << figureName << "' no encontrada en caché.\n";
    return false;
  }

  // Verificamos que el bloque tenga datos
  uint8_t chunkIdx = 0;
  char tmp[CACHE_BLOCK_SIZE];
  uint16_t n = vmm.readBlock(block, tmp, CACHE_BLOCK_SIZE, &chunkIdx);
  if (n == 0) {
    std::cerr << "Error: figura '" << figureName << "' encontrada pero sin datos válidos.\n";
    return false;
  }

  return true;
}

bool CacheManager::store(const std::string& figureName, const std::string& content) {
    ensureFigureIndex(figureName);
    std::vector<int>& blocks = indexTable[figureName];

    size_t written = 0;
    uint8_t chunkIndex = 0;

    while (written < content.size()) {
        size_t remain = content.size() - written;
        uint16_t toWrite = (remain > CACHE_BLOCK_SIZE) ? CACHE_BLOCK_SIZE : remain;

        char nameBuf[10] = {0};
        std::strncpy(nameBuf, figureName.c_str(), 9);

        int block = vmm.handlePageFault(nameBuf);
        vmm.writeBlock(block, content.data() + written, toWrite, chunkIndex);

        blocks.push_back(block);

        written += toWrite;
        chunkIndex++;
    }

    return !blocks.empty();
}


// reconstruye la figura concatenando sus bloques que lee gracias al indexTable y chunckIndex.
std::string CacheManager::retrieve(const std::string& figureName) {
  std::string result;

  if (!indexTable.count(figureName)) {
    std::cerr << "Error: figura '" << figureName << "' no existe en caché.\n";
    return result;
  }

  if (!indexTable.count(figureName)) {
    return result;
  }
  std::vector<int>& blocks = indexTable[figureName];
  for (int b : blocks) {
    uint8_t chunkIdx = 0;
    char buf[CACHE_BLOCK_SIZE];
    uint16_t n = 0;
    // seguimos leyendo hasta que no haya más datos
    while ((n = vmm.readBlock(b, buf, CACHE_BLOCK_SIZE, &chunkIdx)) > 0) {
      result.append(buf, n);
    }
  }
  return result;
}

bool CacheManager::invalidate(const std::string& figureName) {
  // Si no existe en el indice, no hay nada que invalidar
  if (!indexTable.count(figureName)) {
    std::cerr << "Error: figura '" << figureName << "' no existe en caché.\n";
    return false;
  }

  std::vector<int>& blocks = indexTable[figureName];

  // Borrado logico de todos los bloques conocidos
  static const char kZero = 0;
  size_t i = 0;
  while (i < blocks.size()) {
    int b = blocks[i];
    vmm.writeBlock(b, &kZero, 0 /*dataSize*/, 0 /*chunkIdx*/);
    i = i + 1;
  }

  // Quitar la figura del indice
  indexTable.erase(figureName);
  return true;
}

std::string CacheManager::listKeys() {
  std::string result;

  // Si no hay claves, retornamos mensaje
  if (indexTable.empty()) {
    return "(cache vacio)";
  }

  // Construimos la lista en un string con saltos de linea
  bool first = true;
  for (auto it = indexTable.begin(); it != indexTable.end(); it++) {
    if (!first) {
      result += "\n";
    }
    result += it->first;
    first = false;
  }

  return result;
}
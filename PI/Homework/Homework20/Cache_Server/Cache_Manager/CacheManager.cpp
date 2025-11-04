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
  if (!isValidName(figureName)) return false;

  // Usamos findBlockNumber() para localizar un bloque de esa figura
  char nameBuf[10] = {0};
  std::strncpy(nameBuf, figureName.c_str(), 9);

  int8_t block = vmm.findBlockNumber(nameBuf);
  if (block < 0) return false;

  // Verificamos que el bloque tenga datos
  uint8_t chunkIdx = 0;
  char tmp[CACHE_BLOCK_SIZE];
  uint16_t n = vmm.readBlock(block, tmp, CACHE_BLOCK_SIZE, &chunkIdx);
  return n > 0;
}

bool CacheManager::store(const std::string& figureName, const std::string& content) {
  // Si ya existia la figura, la invalidamos para sobrescribirla
  if (indexTable.count(figureName)) {
    invalidate(figureName);
  }

  ensureFigureIndex(figureName);
  std::vector<int>& blocks = indexTable[figureName];

  size_t total = content.size();
  size_t written = 0;
  uint8_t chunkIndex = 0;

  while (written < total) {
    size_t remain = total - written;
    uint16_t toWrite;
    if (remain > CACHE_BLOCK_SIZE) {
      toWrite = CACHE_BLOCK_SIZE;
    } else {
      toWrite = (uint16_t)remain;
    }

    // Construir nameBuf (9 chars max + '\0')
    char nameBuf[10];
    int k = 0;
    while (k < 10) { nameBuf[k] = 0; k = k + 1; }
    k = 0;
    while (k < (int)figureName.size() && k < 9) {
      nameBuf[k] = figureName[k];
      k = k + 1;
    }

    int block = vmm.handlePageFault(nameBuf);
    const char* src = content.data() + written;
    vmm.writeBlock(block, src, toWrite, chunkIndex);

    blocks.push_back(block);

    written = written + toWrite;
    chunkIndex = chunkIndex + 1;
  }

  return !indexTable[figureName].empty();
}

// reconstruye la figura concatenando sus bloques que lee gracias al indexTable y chunckIndex.
std::string CacheManager::retrieve(const std::string& figureName) {
  std::string result;

  if (!indexTable.count(figureName)) {
    return result;
  }

  std::vector<uint8_t> chunkIndices;
  std::vector<std::string> datas;

  std::vector<int>& blocks = indexTable[figureName];

  for (size_t i = 0; i < blocks.size(); i++) {
    int b = blocks[i];
    uint8_t chunkIdx = 0;
    char buf[CACHE_BLOCK_SIZE];
    uint16_t n = vmm.readBlock(b, buf, CACHE_BLOCK_SIZE, &chunkIdx);

    if (n > 0) {
      chunkIndices.push_back(chunkIdx);
      datas.push_back(std::string(buf, buf + n));
    }
  }

  // Bubble Sort pero ordenando ambos vectores a la vez
  for (size_t i = 0; i < chunkIndices.size(); i++) {
    for (size_t j = 0; j + 1 < chunkIndices.size(); j++) {
      if (chunkIndices[j] > chunkIndices[j + 1]) {
        std::swap(chunkIndices[j], chunkIndices[j + 1]);
        std::swap(datas[j], datas[j + 1]);
      }
    }
  }

  // Concatenar
  for (size_t i = 0; i < datas.size(); i++) {
    result += datas[i];
  }

  return result;
}

bool CacheManager::invalidate(const std::string& figureName) {
  // Si no existe en el indice, no hay nada que invalidar
  if (!indexTable.count(figureName)) {
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
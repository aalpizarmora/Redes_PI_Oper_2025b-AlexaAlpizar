#pragma once

#include <cstdint>
#include <queue>
#include <cstring>  // << CHANGE: añadido para strcpy/memcpy (consistencia)

const int NUM_LINES = 4;
const int NUM_BLOCKS = 4; // Per each line
const int CACHE_BLOCK_SIZE = 256; // 2^8 bytes
const int PAGE_SIZE = 256;   // 2^8 bytes
const int NUM_PAGES = 256;   // 2^8 virtual pages
const int NUM_FRAMES = 256;  // physical memory frames
const int PHYS_MEM_SIZE = NUM_FRAMES * PAGE_SIZE;
const int BACKING_STORE_SIZE = NUM_PAGES * PAGE_SIZE;

struct PageTableEntry {
  int frameNumber;  // -1 if not in mem
  bool valid;       // true if in phys mem
};

struct CacheEntry {
  char content[CACHE_BLOCK_SIZE]; // Content of the figure (might be incomplete)
};

struct CachePageTable {

  // valid == true  => bloque LIBRE
  // valid == false => bloque OCUPADO
  bool valid;
  char figureName [10];
  // uint8_t blockNumber; // We discompose this to get the cache line and the offset
  uint8_t usedBlocks; // Generally its 1, but it can be more if the figure is bigger than 256 bytes
  uint8_t  chunkIndex; // orden del fragmento dentro de la figura
  uint16_t chunkSize;  // bytes validos en content (<= 256)
};

class VirtualMemoryManager {
private:
  CacheEntry cache[NUM_LINES][NUM_BLOCKS];
  CachePageTable cacheTable[NUM_LINES*NUM_BLOCKS]; // We have 16 possible entries for the caché

  // PageTableEntry pageTable[NUM_PAGES];
  // int8_t physMem[PHYS_MEM_SIZE];
  // int8_t backingStore[BACKING_STORE_SIZE];

  uint8_t nextFreeBlock;
  // int pageFaultCount;
  // int addressCount;

  // FIFO when using less than 256 frames
  std::queue<int> fifoQueue;

  void initPageTable();
  void loadBackingStore(const char* filename);
  // void extractLineAndOffset(uint8_t blockNumber, uint8_t* cacheLine, uint8_t* offset);
  // int8_t handlePageFault(char* figureName);
  // int8_t findBlockNumber(char* figureName);

public:
  VirtualMemoryManager(const char* backingStoreFile);
  VirtualMemoryManager();
  void extractLineAndOffset(uint8_t blockNumber, uint8_t* cacheLine, uint8_t* offset);
  int8_t handlePageFault(char* figureName);
  int8_t findBlockNumber(char* figureName);

  int translateAddress(char* figure);
  bool writeBlock(int blockNumber, const char* data, uint16_t dataSize, uint8_t chunkIndex);
  uint16_t readBlock (int blockNumber, char* outBuffer, uint16_t maxSize, uint8_t* chunkIndexOut);

  // void printStats();
};

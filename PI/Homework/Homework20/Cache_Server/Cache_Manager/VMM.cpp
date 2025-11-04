#include "VMM.h"
#include <fstream>
#include <iostream>
#include <cstring>
using namespace std;

VirtualMemoryManager::VirtualMemoryManager(const char* backingStoreFile)
  : nextFreeBlock(0) {
  initPageTable();
}

VirtualMemoryManager::VirtualMemoryManager()
  : nextFreeBlock(0) {
  initPageTable();
}

void VirtualMemoryManager::initPageTable() {
  for (int i = 0; i < NUM_LINES * NUM_BLOCKS; i++) {
    cacheTable[i].valid = true;
    strcpy(cacheTable[i].figureName, "");
    cacheTable[i].usedBlocks = 0;
    cacheTable[i].chunkIndex = 0;
    cacheTable[i].chunkSize  = 0;
  }

  for (int L = 0; L < NUM_LINES; L++) {
    for (int O = 0; O < NUM_BLOCKS; O++) {
      memset(cache[L][O].content, 0, CACHE_BLOCK_SIZE);
    }
  }

  while (!fifoQueue.empty()) fifoQueue.pop();
  nextFreeBlock = 0;
}

void VirtualMemoryManager::extractLineAndOffset(uint8_t blockNumber, uint8_t* cacheLine, uint8_t* offset) {
  *cacheLine = blockNumber / NUM_BLOCKS;
  *offset    = blockNumber % NUM_BLOCKS;
}

int8_t VirtualMemoryManager::handlePageFault(char* figureName) {
  int8_t blockNumber;

  if (nextFreeBlock < NUM_LINES * NUM_BLOCKS) {
    blockNumber = nextFreeBlock;
    nextFreeBlock++;
  } else {
    int8_t blockToReplace = fifoQueue.front();
    fifoQueue.pop();

    cacheTable[blockToReplace].valid = true;
    strcpy(cacheTable[blockToReplace].figureName, "");
    cacheTable[blockToReplace].usedBlocks = 0;
    cacheTable[blockToReplace].chunkIndex = 0;
    cacheTable[blockToReplace].chunkSize  = 0;

    uint8_t L = 0, O = 0;
    extractLineAndOffset(blockToReplace, &L, &O);
    memset(cache[L][O].content, 0, CACHE_BLOCK_SIZE);

    cout << "Reemplazando bloque: " << (int)blockToReplace << endl;
    blockNumber = blockToReplace;
  }

  strcpy(cacheTable[blockNumber].figureName, figureName);

  cacheTable[blockNumber].valid = false;
  fifoQueue.push(blockNumber);

  return blockNumber;
}

int8_t VirtualMemoryManager::findBlockNumber(char* figureName) {
  for (int i = 0; i < NUM_LINES * NUM_BLOCKS; i++) {
    if (cacheTable[i].valid == false && strcmp(cacheTable[i].figureName, figureName) == 0) {
      return i;
    }
  }
  return -1;
}

int VirtualMemoryManager::translateAddress(char* figureName) {
  int8_t blockNumber = findBlockNumber(figureName);

  if (blockNumber == -1) {
    blockNumber = handlePageFault(figureName);
  }

  uint8_t cacheLine = 0, offset = 0;
  extractLineAndOffset(blockNumber, &cacheLine, &offset);

  int physAddr = (cacheLine * CACHE_BLOCK_SIZE) + (offset * CACHE_BLOCK_SIZE);
  return physAddr;
}

bool VirtualMemoryManager::writeBlock(int blockNumber, const char* data, uint16_t dataSize, uint8_t chunkIndex) {
  if (blockNumber < 0 || blockNumber >= NUM_LINES * NUM_BLOCKS) return false;
  if (dataSize > CACHE_BLOCK_SIZE) dataSize = CACHE_BLOCK_SIZE;

  uint8_t L = 0, O = 0;
  extractLineAndOffset(blockNumber, &L, &O);

  memset(cache[L][O].content, 0, CACHE_BLOCK_SIZE);
  memcpy(cache[L][O].content, data, dataSize);

  cacheTable[blockNumber].chunkIndex = chunkIndex;
  cacheTable[blockNumber].chunkSize  = dataSize;
  cacheTable[blockNumber].usedBlocks = 1;

  return true;
}

uint16_t VirtualMemoryManager::readBlock(int blockNumber, char* outBuffer, uint16_t maxSize, uint8_t* chunkIndexOut) {
  if (blockNumber < 0 || blockNumber >= NUM_LINES * NUM_BLOCKS) return 0;

  uint8_t L = 0, O = 0;
  extractLineAndOffset(blockNumber, &L, &O);

  uint16_t n = cacheTable[blockNumber].chunkSize;
  if (n > maxSize) n = maxSize;

  if (n > 0) {
    memcpy(outBuffer, cache[L][O].content, n);
  }
  if (chunkIndexOut) *chunkIndexOut = cacheTable[blockNumber].chunkIndex;

  return n;
}

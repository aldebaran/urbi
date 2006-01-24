#include "memorymanager.h"
namespace MemoryManager {
  int allocatedMemory = 0;
};

#ifdef ENABLE_BLOCKMEMMNGR
void block_operator_delete(std::list<void*>  &mempool, void * ptr, int sz) {
  mempool.push_back(ptr);
  MemoryManager::allocatedMemory-= sz;
  
}


void * block_operator_new(std::list<void*>  &mempool, int blocksize, size_t sz) {
  /*if (!mempool) {
    mempool = new std::list<void *>();
  }*/
  if (mempool.empty()) {
    //realloc
    int asz = sz;
    if (asz%8)
      asz = asz+8-(asz%8);
    char * data = (char * )malloc(asz * blocksize);
    for (int i=0;i<blocksize;i++)
      mempool.push_back(data+asz*i);
  }
  void * result = mempool.back();
  mempool.pop_back();
  MemoryManager::allocatedMemory += sz;
  return result;
}
#endif

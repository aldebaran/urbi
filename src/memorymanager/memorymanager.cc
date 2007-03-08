#include <cstdlib>
#include <iostream>
#include "kernel/memorymanager.hh"

namespace MemoryManager
{
  int allocatedMemory = 0;
}


#ifndef DISABLE_BLOCKMEMMNGR

BlockPool::BlockPool ()
  : ptr (0), cptr (0), size (0), itemSize (0)
{
}

void
block_operator_delete(BlockPool* mempool, void* ptr)
{
  mempool->lock();
  ++mempool->cptr;
  *mempool->cptr = ptr;
  MemoryManager::allocatedMemory -= mempool->itemSize;
  mempool->unlock();
}

// This implementation can't release any memory to malloc.
void* block_operator_new(BlockPool* &mempool, int sz)
{
  if (!mempool)
  {
    mempool = new BlockPool;
    mempool->lock();
    --mempool->cptr;
    const int align = sizeof (void*);
    int asz = sz;
    if (asz % align)
      asz = asz + align - (asz % align);
    mempool->itemSize = asz;
  }
  else
    mempool->lock();

  if (!mempool->ptr || mempool->cptr < mempool->ptr)
  {
    int newsize = (mempool->size * 3) / 2 + DEFAULT_BLOCK_SIZE;

    //realloc ptr pool

    //save cptr as relative int
    ptrdiff_t cpos = mempool->cptr - mempool->ptr;
    mempool->ptr =
      static_cast<void**>(realloc (mempool->ptr, newsize * sizeof (void*)));
    mempool->cptr = mempool->ptr + cpos; //restore cptr

    //allocate new data bloc
    char* data = static_cast<char *> (malloc ((newsize-mempool->size)
					       * mempool->itemSize));
    //std::cerr <<"alloc of size "<<newsize<<" "<<(void*)data<<std::endl;
    //std::cerr << mempool->itemSize<<" for "<<sz<<std::endl;
    //std::cerr <<"pool base: "<<mempool->ptr<<std::endl;
    for (int i = 0; i < newsize-mempool->size; ++i)
    {
      ++mempool->cptr;
      *mempool->cptr = data + mempool->itemSize * i;
    }
    mempool->size = newsize;
  }

  void* result = *mempool->cptr;
  --mempool->cptr;
  MemoryManager::allocatedMemory += sz;
  mempool->unlock();
  return result;
}
#endif // !DISABLE_BLOCKMEMMNGR

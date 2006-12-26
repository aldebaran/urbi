#include <cstdlib>
#include <iostream>
#include "memorymanager/memorymanager.hh"

namespace MemoryManager
{
  int allocatedMemory = 0;
}

BlockPool::BlockPool ()
  : ptr (0), cptr (0), size (0), itemSize (0)
{
}

#ifndef DISABLE_BLOCKMEMMNGR
void
block_operator_delete(BlockPool* mempool, void* ptr)
{
  ++mempool->cptr;
  *mempool->cptr = ptr;
  MemoryManager::allocatedMemory -= mempool->itemSize;
}

// This implementation can't release any memory to malloc.
void* block_operator_new(BlockPool* &mempool, int sz)
{
  if (!mempool)
  {
    mempool = new BlockPool;
    --mempool->cptr;
    const int align = sizeof (void*);
    int asz = sz;
    if (asz % align)
      asz = asz + align - (asz % align);
    mempool->itemSize = asz;
  }

  if (!mempool->ptr || mempool->cptr < mempool->ptr)
  {
    int newsize = (mempool->size * 3) / 2 + DEFAULT_BLOCK_SIZE;

    //realloc ptr pool

    //save cptr as relative int
    long cpos = (long) mempool->cptr - (long) mempool->ptr;
    mempool->ptr = static_cast<void**> (realloc (mempool->ptr,
						 newsize * sizeof (void*)));
    mempool->cptr = (void**) ((long) mempool->ptr + cpos); //restore cptr

    //allocate new data bloc
    char* data = static_cast<char *> (malloc ((newsize-mempool->size)
					       * mempool->itemSize));
    //std::cerr <<"alloc of size "<<newsize<<" "<<(void*)data<<std::endl;
    //std::cerr << mempool->itemSize<<" for "<<sz<<std::endl;
    //std::cerr <<"pool base: "<<mempool->ptr<<std::endl;
    for (int i = 0; i < newsize-mempool->size; ++i)
    {
      ++mempool->cptr;
      *mempool->cptr = (data + mempool->itemSize * i);
    }
    mempool->size = newsize;
  }

  void* result = *mempool->cptr;
  --mempool->cptr;
  MemoryManager::allocatedMemory += sz;
  return result;
}
#endif // !DISABLE_BLOCKMEMMNGR

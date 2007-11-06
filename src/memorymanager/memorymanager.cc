#include <cstdlib>
#include <iostream>
#include "kernel/memorymanager.hh"

namespace MemoryManager
{
  size_t allocatedMemory = 0;
}


#ifndef DISABLE_BLOCKMEMMNGR

BlockPool::BlockPool ()
  : ptr (0), cptr (0), size (0), itemSize (0)
{
}

void
block_operator_delete(BlockPool* mempool, void* ptr)
{
  boost::mutex::scoped_lock lock(mempool->mutex);
  ++mempool->cptr;
  *mempool->cptr = ptr;
  MemoryManager::allocatedMemory -= mempool->itemSize;
}

// This implementation can't release any memory to malloc.
void* block_operator_new(BlockPool* &mempool, size_t sz)
{
  const bool blockPoolExist = !!mempool;

  if (!blockPoolExist)
    mempool = new BlockPool;

  boost::mutex::scoped_lock lock(mempool->mutex);

  if (!blockPoolExist)
    {
      --mempool->cptr;
      const int align = sizeof (void*);
      size_t asz = sz;
      if (asz % align)
        asz = asz + align - (asz % align);
      mempool->itemSize = asz;
    }

  if (!mempool->ptr || mempool->cptr < mempool->ptr)
  {
    size_t newsize = (mempool->size * 3) / 2 + DEFAULT_BLOCK_SIZE;

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
    for (size_t i = 0; i < newsize-mempool->size; ++i)
    {
      ++mempool->cptr;
      *mempool->cptr = data + mempool->itemSize * i;
    }
    mempool->size = newsize;
  }

  void* result = *mempool->cptr;
  --mempool->cptr;
  MemoryManager::allocatedMemory += sz;
  return result;
}
#endif // !DISABLE_BLOCKMEMMNGR

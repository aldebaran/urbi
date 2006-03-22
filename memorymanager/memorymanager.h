#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
/**
 * Memory manager common definitions
 */
namespace MemoryManager {
  extern int allocatedMemory; ///< Allocated total
  /* later
     void * alloc(int sz);
     void  free(void * );
     void * realloc(void * addr, int sz);*/
};


#ifdef ENABLE_BLOCKMEMMNGR
#include "blockmemorymanager.h"
#else
#define  MEMORY_MANAGED 
#define MEMORY_MANAGER_INIT(classname)
#endif

#endif

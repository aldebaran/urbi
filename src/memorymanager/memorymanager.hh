#ifndef MEMORYMANAGER_HH
# define MEMORYMANAGER_HH

/**
 * Memory manager common definitions
 */
namespace MemoryManager
{
  extern int allocatedMemory; ///< Allocated total
  /* later
     void * alloc(int sz);
     void  free(void * );
     void * realloc(void * addr, int sz);*/
}


# ifndef DISABLE_BLOCKMEMMNGR
#  include "memorymanager/blockmemorymanager.hh"
# else
#  define  MEMORY_MANAGED
#  define MEMORY_MANAGER_INIT(classname)
# endif

#endif

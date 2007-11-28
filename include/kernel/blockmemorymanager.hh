/*
 * Block memory manager.
 * Class containing MEMORY_MANAGED in their definition will have their
 * instances allocated/freed using the block allocator
 */

#ifndef BLOCKMEMORYMANAGER_HH
# define BLOCKMEMORYMANAGER_HH

# if ! defined URBI_ENV_AIBO
#  include <boost/thread.hpp>
# endif

# define DEFAULT_BLOCK_SIZE 100

/* The placement new is required on the Aibo to support
   std::list<UString>.  */

# define MEMORY_MANAGED				\
  void* operator new(size_t sz)			\
  {						\
    return block_operator_new(mempool_, sz);	\
  }						\
						\
  void* operator new(size_t, void* ptr)		\
  {						\
    return ptr;					\
  }						\
						\
  void operator delete (void *ptr)		\
  {						\
    block_operator_delete(mempool_, ptr);	\
  }						\
						\
  static BlockPool*  mempool_

# define MEMORY_MANAGER_INIT(classname)		\
  BlockPool* classname::mempool_ = 0

class BlockPool
{
 public:
  BlockPool ();

  /// Pool of pointers to free items.
  void** ptr;
  /// Current position in pool.
  void** cptr;
  /// Pool size in number of items.
  size_t size;
  size_t itemSize;

# if ! defined URBI_ENV_AIBO
  boost::mutex mutex;
# endif
};

void block_operator_delete(BlockPool* mempool, void* ptr);

void* block_operator_new(BlockPool* &mempool, size_t sz);

#endif // !BLOCKMEMORYMANAGER_HH

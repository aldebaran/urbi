/*
 * Block memory manager.
 * Class containing MEMORY_MANAGED in their definition will have their
 * instances allocated/freed using the block allocator
 */

#define DEFAULT_BLOCK_SIZE 100

#define MEMORY_MANAGED				\
  void* operator new(size_t sz)			\
  {						\
    return block_operator_new(mempool_, sz);	\
  }						\
						\
  void operator delete (void *ptr)		\
  {						\
    block_operator_delete(mempool_, ptr);	\
  }						\
						\
  static BlockPool*  mempool_

#define MEMORY_MANAGER_INIT(classname)		\
  BlockPool* classname::mempool_ = 0

class BlockPool
{
 public:
  BlockPool ();

  void** ptr;  //pool of pointers to free items
  void** cptr; //curreent position in pool
  int size; //pool size in number of items
  int itemSize;
};
void block_operator_delete(BlockPool* mempool, void * ptr);

void * block_operator_new(BlockPool* &mempool, int sz);

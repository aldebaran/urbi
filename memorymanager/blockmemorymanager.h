/*
 * Block memory manager.
 * Class containing MEMORY_MANAGED in their definition will have their
 * instances allocated/freed using the block allocator
 */
#include <list>
#define DEFAULT_BLOCK_SIZE 50

#define MEMORY_MANAGED void * operator new(size_t sz) \
{return block_operator_new(mempool_,DEFAULT_BLOCK_SIZE, sz);} \
void operator delete (void *ptr, size_t bytes) \
{block_operator_delete(mempool_, ptr, bytes);} \
static std::list<void *> * mempool_;

#define MEMORY_MANAGER_INIT(classname) std::list<void*>* classname::mempool_ = 0;

void block_operator_delete(std::list<void*> * &mempool, void * ptr, int sz);

void * block_operator_new(std::list<void*> * &mempool, int blocksize, size_t sz);

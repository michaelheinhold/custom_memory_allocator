#include <iostream>

using namespace std;

const size_t POOL_SIZE  = 10000;  // total bytes in pool
const size_t BLOCK_SIZE = 32;     // size of each fixed slot
const size_t NUM_BLOCKS = POOL_SIZE / BLOCK_SIZE;

struct FreeBlock
{
  // next is an address
  FreeBlock* next;
};

class PoolAllocator {
  private:
    char pool[POOL_SIZE];             // "heap" access
    // stores the head of the free block list
    FreeBlock* free_list_head = nullptr;
  public:
    void init_pool();
    void* my_alloc();
    void my_free(void* ptr);
};


// walks through pool turning each slot into a linked list of "free slots"
void PoolAllocator::init_pool() {
  for (size_t i = 0; i < NUM_BLOCKS; i++) {
    // steps through BLOCK_SIZE bytes (indexes) in pool array grabbing the first byte's address
    char* block_addr = pool + (i * BLOCK_SIZE);
    // sets block to be a FreeBlock pointer at block_addr
    FreeBlock* block = reinterpret_cast<FreeBlock*>(block_addr);
    // setting next and head of the free list linked list
    block->next = free_list_head;
    free_list_head = block; // remember block is a ptr
  }
}

// reutrns a void pointer so we can work with any data type stored at the memory address
void* PoolAllocator::my_alloc() {
  if (free_list_head == nullptr) {  // this would be the case when all free blocks are used
    cout << "No memory remaining" << endl;
    return nullptr;                 // meaning the program is out of memory
  }
  FreeBlock* block = free_list_head;
  free_list_head = free_list_head->next;
  return block; // returns the starting address of the block of mem to be used
}

// pushes the newly freed block to the front of the linked list
void PoolAllocator::my_free(void* ptr) {
  // turn void ptr into a FreeBlock ptr so we can actually work with it
  FreeBlock* block = reinterpret_cast<FreeBlock*>(ptr);
  block->next = free_list_head;
  free_list_head = block;
}

int main () {
  PoolAllocator pool;
  pool.init_pool();
  int* sixty_nine = reinterpret_cast<int*>(pool.my_alloc());
  *sixty_nine = 69;

  cout << "Address 1: " << sixty_nine << " Value: " << *sixty_nine << endl;

  pool.my_free(sixty_nine);

  cout << "Freed." << endl;

  int* four_twenty = reinterpret_cast<int*>(pool.my_alloc());
  *four_twenty = 420;

  cout << "Address 2: " << four_twenty << " Value: " << *four_twenty << endl;

  cout << (sixty_nine == four_twenty ? "Same address" : "NOT the same address") << endl;

  // I need to allocate EVERY block, then try to allocate another. See what happens
  // First though I need to free the previously used
  pool.my_free(four_twenty);
  int count = 0;
  while (pool.my_alloc() != nullptr) count++;

  cout << "Allocated " << count << " blocks. Expected max to be: " << NUM_BLOCKS <<  endl;

  return 0;
}
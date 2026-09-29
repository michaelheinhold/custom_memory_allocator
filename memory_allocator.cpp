#include <iostream>

using namespace std;

const size_t POOL_SIZE = 10000;

struct Block {
  Block* next;
  size_t size;
};

struct FreeBlock : Block {
};

struct MemBlock : Block {
};

// | FREE | -> |data (next->FREE) | FREE |
// [0, 10000] i need 7 -> [7, 10k] well i need 20 => [27, 10k]
// i dont need the 7 anymore -> [0,6,27,10k]

class MemAllocator {
private:
  // array of char (1 byte) at the size 10,000 as set above. can look into dynamically allocationg this, however for now
  /// we'll stick to dynamic allocation of the BLOCKS
  char pool[POOL_SIZE];
public:
  FreeBlock* free_list_head = nullptr;
  MemAllocator();
  void* my_alloc(size_t size);
  void my_free(void* ptr);
  //~MemAllocator();
};

// constructor should just initialize the pool
MemAllocator::MemAllocator() {
  FreeBlock* free_block = reinterpret_cast<FreeBlock*>(pool);
  free_block->size = POOL_SIZE;
  free_list_head = free_block;
}

void* MemAllocator::my_alloc(size_t size) {
  // get the address that will be at the END of this block - this will be the new head of the free block we're
  // breaking up - We'll store the size in the header, this should be all thats needed. and return the address AFTER
  // the header
  // THOUGHTS - pool always starts at index 0 of pool, perhaps instead of adding pool we use free_list_head?
  // we would also need to add a check that free_list_head has enough space, if not check if there is another free_block
  // on its ->next, if it's nullptr we must be out of memory. IF there is one, we still need to run the check of if it's
  // large enough. Sounds like the work of a while loop?
  size_t header = size; //this may be unnecessary holding for now
  char* block_end_addr = reinterpret_cast<char*>(free_list_head) + (size + sizeof(header)); // this accounts for both header AND allocated block
  // using reinterpret cast here could cause some garbage on the start of the new free block, should just overwrite the size
  // well- we can't use POOL_SIZE maybe this calc needs to be done before we mess up all the data, where to store it though?
  // get through this for now

  // now we have the whole block we set the first bit to header. Then get the the address after header using pool + header
  // return that address
  char* block_start_addr = reinterpret_cast<char*>(free_list_head) + sizeof(header); // should be what we return
  // now store the header at the index
  size_t* temp = reinterpret_cast<size_t*>(free_list_head);

  // new free block
  FreeBlock* new_free_block = reinterpret_cast<FreeBlock*>(block_end_addr);
  new_free_block->size = free_list_head->size - (size + sizeof(size_t));
  free_list_head = new_free_block;

  // set temp to header
  *temp = header;
  // return
  return block_start_addr;
}

void MemAllocator::my_free(void* ptr) {
  // get header info
  FreeBlock* block_addr = reinterpret_cast<FreeBlock*>(ptr);
  char* header = reinterpret_cast<char*>(block_addr) - sizeof(size_t);
  cout << "Address of block header (should match initial head address): " << static_cast<const void*>(header)
    << " | Test ptr carries: "<< block_addr << endl;
  cout << "Can I read the value at header? " << static_cast<const size_t>(*header) << endl;
  // this should spit out the data we added to the header, then we add the header size as well.
  // now we can set the start of the block addr to the free state and check if whats after it is another free block
  size_t size = static_cast<const size_t>(*header) + sizeof(size_t);
  // we check if the next block is free block
  cout << "TEST: " << static_cast<const void*>(header + size) << " FREE: " << free_list_head << " Size: " << size << endl;
  if (static_cast<const void*>(header + size) == free_list_head) {
    // this should just mark the entire block as free and update the size to match - I dont think anyhting else is needed
    size_t new_size = free_list_head->size += size;
    // new_size was needed becuase the reinterpret cast was causing issues with the size that was stored
    // basically just created a temp to hold onto the new size then reassigned it
    // will look for refinement later - this works
    free_list_head = reinterpret_cast<FreeBlock*>(header);
    free_list_head->size = new_size;
    // just to test:
    cout<< free_list_head << " " << free_list_head->size << endl;
  }
  /*
  at this point the program works for adding one allocated block and removing it - No validation,
  no additional blocks.
  This next if will be the first step - what if this is the 0th block and we added a 1st index block?
  Free block will not be the next one so we need to create a second free-block
  there is a couple issues however. I planned on doing a linked list of all the free blocks. But I need to know where
  to add this new one, the order unfortunately matters. I can compare addresses with < but its going to feel
  really clunky because how the fuck do i find the correct index.
  */
}

int main() {
  MemAllocator theHeap;

  cout << "Free block head address: " << theHeap.free_list_head << endl;

  int* sixty_nine = reinterpret_cast<int*>(theHeap.my_alloc(64));
  *sixty_nine = 69;

  cout << "Sixty Nine is 64 bytes. Start address: " << sixty_nine << " | New free block head address: " << theHeap.free_list_head << endl;
  cout << "Free space shoud be 10k-72 now (9928), here is whats stored: " << theHeap.free_list_head->size << endl;

  theHeap.my_free(sixty_nine); // should cout 64

  return 0;
}

/*
taking a break at 12:29pm notes:
I've got the allocation working for now. Yet to test double allocation, first I'm working on
freeing up the one block at a time, which is where I'm stuck with how Im actually reading the data
I need to verify the header start address is correct, and that im starting in the right spot
This forum seems to cover how chars are treated in cout and it might be relevant
https://stackoverflow.com/questions/17813423/cout-with-char-argument-prints-string-not-pointer-value
UPDATE the info from the link worked, i static cast the char* into void* because char* are weird. Couldn't
really say more on why that works or what it's doing. Hopefully not breaking anything.
*/
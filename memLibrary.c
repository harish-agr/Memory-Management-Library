#include <stdio.h>
#include <semaphore.h>

typedef unsigned char byte;
typedef struct{
  int size;
  int alloc;
} tableEntry_t;

static sem_t memLock;


// Base pointer to initial memory address
void *memPoint;

// Table of memory addresses, each entry will either be
// 0 for unallocated or 1 for allocated
tableEntry_t *memTable;

// The total size in bytes of the memory to work with
int memSize;


// Allocates a memory partition with "size" bytes. Returns
// a pointer to the memory space, or NULL if there is no space
void *get_memory(int size){
  // Go through memTable and find first empty block.
  // Check that there is space equal to "size" after
  // the first empty block.
  int found = 0;
  int index = 0;
  sem_wait(&memLock);
  for (int i=0 ; i < memSize; i++){
    found = 0;
    // We have found an unallocated block. If there is
    // sufficient room left in our memory, check if there
    // is sufficient unallocated space
    if (memTable[i].alloc==0 && i + size <= memSize){
      found = 1;
      for (int j = i; j < i + size; j++){
        if (memTable[j].alloc==1){
          found = 0;
          break;
        }
      }
      if (found == 1){
        // Store the first index in "found"
        index = i;
        break;
      }
    }
  }

  // If there was no partition of memory found, return null
  if (found == 0){
    return NULL;
  }

  // Otherwise, return a pointer to the first index and mark
  // all of the blocks as allocated.
  memTable[index].size = size;
  for (int i = index; i < index + size; i++){
    memTable[i].alloc = 1;
  }
  sem_post(&memLock);
  return memPoint + index;
}

// Attempts to grow the memory partition referenced by
// p to number of bytes equal to size. If it is not
// possible to grow the partition in place, but there
// is enough space elsewhere, the memory is copied to
// the new partition. If the partition cannot be grown,
// NULL is returned. If the value of size is smaller
// than the partition's current size, bytes are truncated
// from the end of the partition to shrink it down.
void *grow_memory(int size, void *p){
  int index = ((int)p - (int)memPoint);
  int sizeOfBlock = memTable[index].size;

  // The case that we are growing the partition
  if (sizeOfBlock < size){
    // Check if there is enough space to grow in place
    int growInPlace = 0;
    if (index + size < memSize){
      growInPlace = 1;
      for (int i = index+sizeOfBlock; i < index + size; i++){
        if (memTable[i].alloc==1){
          growInPlace = 0;
          break;
        }
      }
    }
    if (growInPlace == 1){
      sem_wait(&memLock);
      // Update the size
      memTable[index].size = size;
      // Mark the additional blocks as allocated
      for (int i = index+sizeOfBlock; i < index + size; i++){
        memTable[i].alloc = 1;
      }
      sem_post(&memLock);
      return p;
    }
    // Otherwise, need to find a new location in memory
    else{
      void *q = get_memory(size);
      // Check if there is enough memory elsewhere
      if (q == NULL){
        return NULL;
      }
      release_memory(p);
      sem_wait(&memLock);
      // Copy over the contents referenced by p to
      // the new space referenced by q
      byte* tmpTo = q;
      byte* tmpFrom = p;
      for (int i = 0; i < sizeOfBlock; i++){
        *tmpTo = *tmpFrom;
        tmpTo++;
        tmpFrom++;
      }
      // Free the contents of p
      memTable[((int)q-(int)memPoint)].size=size;
      sem_post(&memLock);
      return q;
    }
  }
  // The case that we are shrinking the partition
  else if (sizeOfBlock > size){
    if (size == 0){
      release_memory(p);
      return NULL;
    }
    sem_wait(&memLock);
    for (int i = index + size; i < index + sizeOfBlock; i++){
      memTable[i].alloc=0;
    }
    sem_post(&memLock);
    return p;
  }
  // The case that the current and new sizes are
  // the same
  else{
    printf("The current and new size are the same\n");
    return p;
  }
  return NULL;
}

// This function behaves the same as grow memory, except
// any space allocated is allocated to indices lower than
// the partition's base index, instead of higher indices
void *pregrow_memory(int size, void *p){
  int index = ((int)p - (int)memPoint);
  int sizeOfBlock = memTable[index].size;

  // The case that we are growing the partition
  if (sizeOfBlock < size){
    // Check if there is enough space to grow in place
    int growInPlace = 0;
    if (index - size + sizeOfBlock >= 0){
      growInPlace = 1;
      for (int i = index-1; i >= index - size + sizeOfBlock; i--){
        if (memTable[i].alloc==1){
          growInPlace = 0;
          break;
        }
      }
    }
    // If there is enough space to grow in place,
    // mark all blocks as allocated, shift the
    // contents of memory back, and return a pointer
    // to the new base index
    if (growInPlace == 1){
      sem_wait(&memLock);
      memTable[index].size = 0;
      index = index - size + sizeOfBlock;
      // Update the size
      memTable[index].size = size;
      // Mark the additional blocks as allocated
      for (int i = index; i < index + size; i++){
        memTable[i].alloc = 1;
      }
      // Shift the contents of memory back
      byte* tmpTo = memPoint + index;
      byte* tmpFrom = p;
      for (int i = 0; i < size; i++){
        *tmpTo = *tmpFrom;
        tmpTo++;
        tmpFrom++;
      }
      sem_post(&memLock);
      return memPoint + index;
    }
    // Otherwise, need to find a new location in memory
    else{
      void *q = get_memory(size);
      // Check if there is enough memory elsewhere
      if (q == NULL){
        return NULL;
      }
      // Copy over the contents referenced by p to
      // the new space referenced by q
      sem_wait(&memLock);
      byte* tmpTo = q;
      byte* tmpFrom = p;
      for (int i = 0; i < sizeOfBlock; i++){
        *tmpTo = *tmpFrom;
        tmpTo++;
        tmpFrom++;
      }
      memTable[((int)q-(int)memPoint)].size=size;
      sem_post(&memLock);
      // Free the contents of p
      release_memory(p);
      return q;
    }
  }
  // The case that we are shrinking the partition
  else if (sizeOfBlock > size){
    if (size == 0){
      release_memory(p);
      return NULL;
    }
    sem_wait(&memLock);
    for (int i = index; i < index + sizeOfBlock - size; i++){
      memTable[i].alloc=0;
    }
    sem_post(&memLock);
    return p + (sizeof(p)*(sizeOfBlock-size));
  }
  // The case that the current and new sizes are
  // the same
  else{
    printf("The current and new size are the same\n");
    return p;
  }

  return NULL;
}

// Releases the partitions referenced by p
// back into the pool of free memory
void release_memory(void *p){
  int index = ((int)p - (int)memPoint);
  int sizeOfBlock = memTable[index].size;

  sem_wait(&memLock);
  // Set the entry's size to 0
  memTable[index].size = 0;

  // Go through the table and mark all blocks as free
  for (int i = index; i < index + sizeOfBlock; i++){
    memTable[i].alloc = 0;
  }
  sem_post(&memLock);
}

// Stores the size of the memory pool, allocates space
// for the table and the pool, sets all entries to 0 in
// the table, and returns either 1 for success or 0 for
// failure. Also initializes the semaphore to protect memory
int start_memory(int size){
  // Store size so it can be referenced later
  memSize = size;

  // Initialize the semaphore
  sem_init(&memLock, 0, 1);

  // Allocate number of bytes equal to size. Later on this
  // should be changed to allocate enough for internal data
  // structures as well.
  memPoint = malloc(size);
  memTable = malloc(sizeof(tableEntry_t)*size);
  printf("Base pointer pointing to: %d\n", (int)memPoint);

  // Mark all entries in memTable as unallocated
  for (int i=0 ; i < memSize; i++){
    memTable[i].size = 0;
    memTable[i].alloc = 0;
  }

  if (memPoint == NULL){
    return 0;
  }
  return 1;
}

// Prints out all blocks which were not released, and then
// frees the memory allocated for the table and pool
// Also frees the semaphore
void end_memory(void){
  for (int i = 0; i < memSize; i++){
    if (memTable[i].alloc==1){
      printf("Block %d was not released\n", i);
    }
  }
  sem_destroy(&memLock);
  free(memTable);
  free(memPoint);
}

// Prints the current state of the memory table
void print_table(){
  for (int i = 0; i < memSize; i++){
    printf("Table Entry: %d\tis: %d\n", i, memTable[i].alloc);
  }
  printf("\n");
}

// This is a test program which I used to help find bugs and 
// test different features. It produces some unpleasant output 
// such as printing out the contents of the memory table a few 
// times, but it demonstrates the various functions of my memory
// management library fairly well. I have intentionally not
// released all of my allocated memory so that it will display
// the leaks at the end of the program. I assume that it is all
// cleaned up during the final call to free() anyhow, but
// I wanted to show detection of leaks just the same.

#include <stdio.h>
#include "memLibrary.h"
#include <pthread.h>

void handler (void *ptr);
pthread_t thread1;
pthread_t thread2;


int main(){
  start_memory(sizeof(int)*24);
  int choice = 0;


  printf("0: Make 3 partitions, remove middle, shrink in place\n");
  printf("1: Make 3 partitions, remove middle, grow in place, grow by moving\n");
  printf("2: Make 3 partitions, remove middle, pregrow in place, pregrow by moving\n");
  printf("3: Test implementation of semaphores\n");
  printf("Which test to run?: ");
  scanf("%d", &choice);

  // Create 3 partitions of size 2
  int *p = get_memory(sizeof(int)*2);
  printf("pointer returned: %d\n", (int)p);
  p[0] = 4;
  p[1] = 6;

  int *q = get_memory(sizeof(int)*2); 
  printf("pointer returned: %d\n", (int)q);

  int *r = get_memory(sizeof(int)*2);
  printf("pointer returned: %d\n", (int)r);
  r[0] = 2;
  r[1] = 3;

  switch(choice){
    case 0:   // Remove the middle partition
              release_memory(q);
              print_table();
            
              //Attempt to shrink memory
              p = grow_memory(1, p);
              print_table();


              break;
    case 1:   // Remove the middle partition
              release_memory(q);
              print_table();


              // Attempt to grow memory in place
              // when there enough space.
              // Print out the value before and 
              // after the memory is copied to a 
              // new partition to make sure it
              // is not corrupted in the transfer
              printf("%d %d\n", p[0], p[1]);
              p = grow_memory(sizeof(int)*4, p);
              printf("%d %d\n", p[0], p[1]);
              print_table();


              // Attempt to grow memory in place
              // when there is not enough space.
              // Print out the value before and 
              // after the memory is copied to a 
              // new partition to make sure it
              // is not corrupted in the transfer
              printf("%d %d\n", p[0], p[1]);
              p = grow_memory(sizeof(int)*8, p);
              printf("%d %d\n", p[0], p[1]);

              print_table();
              break;

    case 2:   // Remove the middle partition
              release_memory(q);
              print_table();

              // Attempt to pregrow r in place
              // and make sure values stored are not 
              // corrupted in the shift to an earlier 
              // location in memory
              printf("%d %d\n", r[0], r[1]);
              r = pregrow_memory(4, r);
              printf("%d %d\n", r[0], r[1]);
              print_table();
              break;

    case 3:   ;
              // Create some values and send them to the threads
              int vars[2];
              vars[0] = 7;
              vars[1] = 8;
              pthread_create(&thread1, NULL, (void*) &handler, (void *) &vars[0]);
              pthread_create(&thread1, NULL, (void*) &handler, (void *) &vars[1]);

              pthread_join(thread1, NULL);
              pthread_join(thread2, NULL);
              print_table();
              break;
    default:  
              break;  
  }
  if (q != NULL){
    release_memory(q);
  }
  release_memory(p);
  release_memory(r);
  end_memory();

  return 1;
}

// Tests semaphores by creating new pointers to an integer and
// storing the values. By locking down the memory while it is
// being modified, multiple threads do not interfere with
// eachother
void handler(void *ptr){
  int modifier;
  modifier = *((int *) ptr);
  int* point = get_memory(sizeof(int));
  point = modifier;
  printf("Printing a value from inside a thread %d\n",point);
  pthread_exit(0);
}



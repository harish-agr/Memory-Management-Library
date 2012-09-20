#ifndef memLibrary
#define memLibrary



void *get_memory(int size);
void *grow_memory(int size, void *p);
void *pregrow_memory(int size, void *p);
void release_memory(void *p);
int start_memory(int size);
void end_memory(void);
void print_table();


#endif

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "hw3.h"

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }


long distance_to_end_of_heap(void * ptr)
{
    // tests 01
    // TODO: PDF -> Distance To End of Heap
    long returnValue;
    //BRK - ptr 
    void* brk1 = sbrk(0);
    returnValue = (char*)brk1 - (char*)ptr;
    return returnValue;
    //look at lab6 for help on the distance
}


long chunk_size(void * ptr)
{
    // tests 02
    // TODO: PDF -> Chunk Size
    //get header, calculate size by getting rid of the 3 least significant bits,
    // appending 0 and converting back to decimal
    //so header = 0x15 = 0001 0101 -> 0001 0-000 -> 0x10 = 16 bytes
    long returnValue;
    char* charptr = (char*) ptr;
    charptr -= 8;//move back 8 bytes to get header
    uintptr_t value = *(uintptr_t*)charptr;
    uintptr_t mask = 0x7;
    returnValue = (value & ~(uintptr_t)mask); //removes 3 least significant bits, use uintptr_t for casting
    // printf("rvalue: %ld\n",returnValue);
    return returnValue;

}



int chunk_in_use(void * ptr)
{
    // tests 03
    // TODO: PDF -> Chunk In Use
    //go to next chunk, find prev in use value,
    long chunksize = chunk_size(ptr);
    char* charptr = (char*)ptr;
    charptr += chunksize;
    charptr -= 8;//move back 8 bytes to get header, may or may not be needed
    uintptr_t value = *(uintptr_t*)charptr;
    uintptr_t mask = 0x1;//0111 = 0x7, 1000 = 0x8
    int returnValue = (value & mask); //removes 3 least significant bits, use uintptr_t for casting
    return returnValue;
}


void * next_used_mem(void * ptr)
{
    // tests 04 & 05
    // given a ptr return next used memory chunk
    // TODO: PDF -> Next Used Mem
    //First get ptr, then go to next header, get size, and go to next header, if the prev in use is true, 
    // return the previous header,
    //  else keep going to next header
    long chunksize = chunk_size(ptr);
    char* second_ptr = ((char*)ptr) + chunksize;//
    // long chunksize2 = chunk_size(second_ptr);
    // char* third_ptr = ((char*)second_ptr) + chunksize2;
    int inUse = chunk_in_use(second_ptr);
    while(inUse == 0) {
        second_ptr+=chunk_size(second_ptr);
        inUse = chunk_in_use(second_ptr);
    }
    return (void*)second_ptr;

    
}


void * next_free_mem(void * ptr)
{

    // tests 06 & 07
    // return next free chunk
    // TODO: PDF -> Next Free Mem
    //Same as next_used_mem() but with free chunk instead

    long chunksize = chunk_size(ptr);
    char* second_ptr = ((char*)ptr) + chunksize;//
    // long chunksize2 = chunk_size(second_ptr);
    // char* third_ptr = ((char*)second_ptr) + chunksize2;
    int inUse = chunk_in_use(second_ptr);
    while(inUse != 0) {
        second_ptr+=chunk_size(second_ptr);
        inUse = chunk_in_use(second_ptr);
    }
    return (void*)second_ptr;

    assert(0);
}


void free_everything(void * start, void * end, int size, long * stats)
{

    // ignoring stats is enough to pass test 08
    // stats are tests 09 and 10
    // finally only free things larger than size
    //stats is a pointer to an array of 2 long elements, 
    // stats[0] == numberOfChunksFreed, 
    // stats[1] == totalSizeOfFreedMemoryInBytes
    //Only frees memory in between start and end, doesn't include start and end


    long how_many_freed = 0;
    long how_many_bytes = 0;

    // TODO: PDF -> Free Everything

    void* currChunk = start;
    currChunk = next_used_mem(currChunk);
    
    while(currChunk != end) {

        long cs = chunk_size(currChunk);
        if(cs > size) {
            void* chunkToFree = currChunk;
            how_many_freed+=1;
            how_many_bytes+=cs;
            free(chunkToFree);

        }
        currChunk = next_used_mem(currChunk);
    }

    if (stats != NULL) {
        stats[0] = how_many_freed;
        stats[1] = how_many_bytes;
    }


    
}

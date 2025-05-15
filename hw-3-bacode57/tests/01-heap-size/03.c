#include "hw3.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main()
{
    // Allocates one chunk
    void * ptr = malloc(4);

    // How far is it from the end of the heap?
    long d1 = distance_to_end_of_heap(ptr);

    // A bunch of allocations now
    for (int i = 0 ; i < 10000 ; i++) {
        malloc(i);
    }

    // Get the distance from the same pointer to the end of the heap
    long d2 = distance_to_end_of_heap(ptr);

    // Print some stuff to help debug
    printf("%lu\n", d1);
    printf("%lu\n", d2);

    printf("%lu\n", d2-d1);

    // The bunch of mallocs should have moved the end of the heap
    assert(d2 > d1);

    return 0;
}

#include "hw3.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main()
{
    // Allocates two chunks
    void * ptr1 = malloc(4);
    void * ptr2 = malloc(4);

    // How far is each from the end of the heap?
    long d1 = distance_to_end_of_heap(ptr1);
    long d2 = distance_to_end_of_heap(ptr2);

    // Print some stuff to help debug
    printf("%lu\n", d1);
    printf("%lu\n", d2);

    printf("%lu\n", d1-d2);
    printf("%lu\n", ptr2-ptr1);

    // The two distances between the end of the heap should be the same as the distances between pointers
    assert((d1-d2) == (ptr2-ptr1));

    // Free used memory
    free(ptr1);
    free(ptr2);
    return 0;
}

#include "hw3.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main() {
    int size = 1;
    int expected = 1 << 5;

    // Allocates one chunk which should use a whole chunk
    void * ptr = malloc(size);

    // Gets the size of the chunk
    long d = chunk_size(ptr);

    // Print some debug stuff
    printf("Allocated size: %d\n", size);
    printf("Reported chunk size: %ld\n", d);
    printf("Expected chunk size: %d\n", expected);

    // Checks that the size reported is positive
    assert(d > 0);

    // Checks that the size reported is not zero
    assert(d != 0);

    // Checks that the size reported is correct
    assert(d == expected);

    // Free used memory
    free(ptr);
    return 0;
}

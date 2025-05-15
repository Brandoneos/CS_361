#include "hw3.h"

#define _DEFAULT_SOURCE
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main() {
    int m_size = 128;

    srandom(time(NULL));

    // Allocates something so that the  heap doesn't become empty
    // Empty heaps can be weird...
    malloc(1000);

    void * data[500];
    int status[500];

    for (int i = 0 ; i < (sizeof(data)/sizeof(*data)) ; i++) {
        // Get a size at random
        int n = 128 + ((int) (random() % 1000));

        // Allocates memory with random size and saves it
        data[i] = malloc(n);

        // Set the status as alloc'ed
        status[i] = 0;
    }

    // Makes sure all data is not free
    for (int i = 0 ; i < (sizeof(data)/sizeof(*data)) ; i++) {
        int is_free = is_chunk_free(data[i]);
        assert(is_free == 0);
    }

    // Frees half data at random
    for (int i = 0 ; i < ((sizeof(data)/sizeof(*data)) >> 1) ; i++) {
        // Get a position at random
        int n = (int) (random() % (sizeof(data)/sizeof(*data)));

        // Make sure it is not free already
        if (status[n] == 1)
            continue;

        // Make sure the previous is alloc'ed
        if (status[n-1] == 1)
            continue;

        // Make sure the next is alloc'ed
        if (status[n+1] == 1)
            continue;

        // Frees that data
        free(data[n]);

        // Set the status as free'ed
        status[n] = 1;
    }

    // Check that next_mem finds all the used data
    for (int i = 0 ; i < ((sizeof(data)/sizeof(*data)) - 2) ; i++) {
        void * ptr = data[i];

        if (status[i])
            // Current mem is free, keep going
            continue;

        // Find next used memory
        void * expected = NULL;
        for (int j = i+1 ; j < ((sizeof(data)/sizeof(*data)) - 2) ; j++) {
            if (!status[j]) {
                expected = data[j];
                break;
            }
        }

        if (expected == NULL)
            // Didn't find used memory, nothing left to do
            break;

        // Asks the API what is the next used memory
        void * actual = next_used_mem(ptr);

        // Should be the same
        assert(actual == expected);
    }

    return 0;
}

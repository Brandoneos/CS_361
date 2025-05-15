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

    // Frees 5 data at random
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

    // Checks that each data is correctly detected as free or used
    for (int i = 1 ; i < ((sizeof(data)/sizeof(*data)) - 2) ; i++) {
        int expected = status[i];
        int actual = is_chunk_free(data[i]);

        if (actual != expected) {
            // Oops something went wrong!  Check the output of the test, it will show you some clues
            for (int j = 1 ; j < ((sizeof(data)/sizeof(*data)) - 2) ; j++) {
                if (j == i)
                    printf("%d %d %d\n", j, status[j], actual);
                else
                    printf("%d %d\n", j, status[j]);
            }
        }

        assert(actual == expected);
    }

    return 0;
}

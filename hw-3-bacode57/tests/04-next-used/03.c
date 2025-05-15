#include "hw3.h"

#define _DEFAULT_SOURCE
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main() {
    // Allocates something so that the  heap doesn't become empty
    // Empty heaps can be weird...
    malloc(1000);

    srandom(time(NULL));

    void * ptrs[1024];

    for (int i = 0 ; i < sizeof(ptrs)/sizeof(*ptrs) ; i++) {
        // Allocate a bunch of memory with random sizes
        int m_size = 128 + ((int) (random() % 1000));
        void * ptr = malloc(m_size);
        ptrs[i] = ptr;
    }

    for (int i = 0 ; i < (sizeof(ptrs)/sizeof(*ptrs) - 1) ; i++) {
        // Make sure that the API always find the next used memory
        void * next = next_used_mem(ptrs[i]);

        assert(next == ptrs[i+1]);
    }

    return 0;
}

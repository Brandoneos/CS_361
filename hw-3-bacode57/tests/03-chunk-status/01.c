#include "hw3.h"

#define _DEFAULT_SOURCE
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main() {
    int m_size = 128;

    // Allocates something so that the  heap doesn't become empty
    // Empty heaps can be weird...
    malloc(1000);

    // Allocates one chunk and fills it with 'a'
    char * p1 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p1[i] = 'a';

    // Allocates one chunk and fills it with 'b'
    char * p2 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p2[i] = 'b';

    // Allocates one chunk and fills it with 'c'
    char * p3 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p3[i] = 'c';

    {
        // Calls the API
        int i1 = is_chunk_free(p1);
        int i2 = is_chunk_free(p2);
        int i3 = is_chunk_free(p3);

        // Makes sure no chunk is not free
        assert(i1 == 0);
        assert(i2 == 0);
        assert(i3 == 0);
    }

    // Frees one chunk
    free(p2);

    {
        // Calls the API
        int i1 = is_chunk_free(p1);
        int i2 = is_chunk_free(p2);
        int i3 = is_chunk_free(p3);

        // Makes sure one chunk is free
        assert(i1 == 0);
        assert(i2 != 0);
        assert(i3 == 0);
    }

    return 0;
}

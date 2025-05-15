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
        void * i2 = next_used_mem(p1);
        void * i3 = next_used_mem(p2);

        // Makes sure we got the right pointers
        assert(i2 == p2);
        assert(i3 == p3);
    }

    return 0;
}

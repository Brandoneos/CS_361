#include "hw3.h"

#define _DEFAULT_SOURCE
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main() {
    srandom(time(NULL));

    // Allocates something so that the  heap doesn't become empty
    // Empty heaps can be weird...
    malloc(1000);

    char *p1, *p2, *p3;

    {
        // Allocates one chunk and fills it with 'a'
        int m_size = 128 + ((int) (random() % 1000));
        printf("Allocating %d\n", m_size);
        p1 = malloc(m_size);

        for (int i = 0; i < m_size; i++)
            p1[i] = 'a';
    }

    {
        // Allocates one chunk and fills it with 'b'
        int m_size = 128 + ((int) (random() % 1000));
        printf("Allocating %d\n", m_size);
        p2 = malloc(m_size);

        for (int i = 0; i < m_size; i++)
            p2[i] = 'b';
    }

    {
        // Allocates one chunk and fills it with 'c'
        int m_size = 128 + ((int) (random() % 1000));
        printf("Allocating %d\n", m_size);
        p3 = malloc(m_size);

        for (int i = 0 ; i < m_size ; i++)
            p3[i] = 'c';
    }

    {
        // Calls the API
        void * i2 = next_used_mem(p1);
        void * i3 = next_used_mem(p2);

        // Makes sure we got the right pointers
        assert(i2 == p2);
        assert(i3 == p3);
    }

    // Frees the middle memory
    free(p2);

    {
        // Calls the API
        void * i3 = next_used_mem(p1);

        // Makes sure we got the right pointers
        assert(i3 != p2);
        assert(i3 == p3);
    }

}

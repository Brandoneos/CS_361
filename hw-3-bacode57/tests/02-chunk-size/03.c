#include "hw3.h"

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

static int expected[] = { 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512, 528, 544, 560, 576, 592, 608, 624, 640, 656, 672, 688, 704, 720, 736, 752, 768, 784, 800, 816, 832, 848, 864, 880, 896, 912, 928, 944, 960, 976, 992, 1008, };


static int find(int * arr, int n, int needle) {
    for (int i = 0 ; i < n ; i++) {
        if (arr[i] == needle)
            return i;
    }

    return -1;
}

int is_chunk_free(void * ptr) { return !chunk_in_use(ptr); }

int main() {

    srandom(time(NULL));

    for (int i = 0 ; i < 100000 ; i++) {

        // Get a size at random
        int n = (int) 1 + (random() % ((sizeof(expected)/sizeof(*expected)) - 1));

        // Allocate a chunk that size
        void * ptr = malloc(expected[n-1]);

        // See what size the chunk is
        int s = (int) chunk_size(ptr);

        // Assert it is the expected size
        assert(s == expected[n]);
    }

    return 0;
}

#include <stdlib.h>
#include "game.h"

#pragma code-name (push, "LOWCODE")

void seed_random(unsigned int seed) {
    srand(seed);
}

unsigned char rand_range(unsigned char min, unsigned char max) {
    unsigned char span;

    if (min >= max) {
        return min;
    }

    span = max - min + 1;
    return min + ((unsigned int)rand() % span);
}

#pragma code-name (pop)

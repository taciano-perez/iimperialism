#include <stdlib.h>
#include "game.h"

#pragma code-name (push, "LOWCODE")

void seed_random(unsigned int seed) {
    srand(seed);
}

unsigned int rand_range(unsigned int min, unsigned int max) {
    unsigned long span;

    if (min >= max) {
        return min;
    }

    span = (unsigned long)(max - min) + 1UL;
    return min + ((unsigned long)rand() % span);
}

#pragma code-name (pop)

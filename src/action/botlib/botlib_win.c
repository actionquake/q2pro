#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <time.h>

void seed_random_number_generator(void) {
    srand((unsigned int)time(NULL));
}
#endif
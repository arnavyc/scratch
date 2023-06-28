#ifndef AY_RANDOM_H
#define AY_RANDOM_H

#include <stddef.h>

enum {
  OSRNG_OK = 0,
  OSRNG_NO_RNG_AVAILABLE = 1,
  OSRNG_INTERNAL_ERROR = 2,
  OSRNG_ERROR_RNDSRC_READ = 3
};

int getrand(size_t buf_size, void *buf);

#endif /* AY_RANDOM_H */

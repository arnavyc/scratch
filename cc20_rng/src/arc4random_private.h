#ifndef AY_ARC4RANDOM_PRIVATE_H
#define AY_ARC4RANDOM_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

#define RNG_POOL_SIZE 768
#define CHACHA20_KEY_SIZE 32
#define RNG_SEED_SIZE CHACHA20_KEY_SIZE

struct RngCtx {
  uint8_t pool[RNG_POOL_SIZE];
  size_t index;
};

struct a4r_rng {
  struct RngCtx rng;
  unsigned char is_seeded;
};

void a4r_explicit_bzero(void *b, size_t len);

int a4r_init_global(void);

struct a4r_rng *a4r_rng_get(void);
void a4r_rng_put(struct a4r_rng *r);

int a4r_getentropy(void *buffer, size_t length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AY_ARC4RANDOM_PRIVATE_H */

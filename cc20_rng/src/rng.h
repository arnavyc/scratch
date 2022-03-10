#ifndef AY_CC20_RNG_H
#define AY_CC20_RNG_H

#include <stddef.h>
#include <stdint.h>

#define RNG_POOL_SIZE 768

#define CHACHA20_KEY_SIZE 32

typedef struct {
  uint8_t pool[RNG_POOL_SIZE];
  size_t index;
} RngCtx;

void rng_init(RngCtx *ctx, const uint8_t seed[CHACHA20_KEY_SIZE]);
void rng_read(RngCtx *ctx, size_t bufsz, void *buf);

#endif /* AY_CC20_RNG_H */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <monocypher.h>

#include "arc4random.h"
#include "arc4random_private.h"

void a4r_explicit_bzero(void *b, size_t len) { crypto_wipe(b, len); }

static void rng_init(struct RngCtx *ctx,
                     const uint8_t seed[CHACHA20_KEY_SIZE]) {
  memcpy(ctx->pool, seed, CHACHA20_KEY_SIZE);
  ctx->index = RNG_POOL_SIZE;
}

static void rng_fill_pool(struct RngCtx *ctx) {
  static const uint8_t zero_nonce[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  crypto_chacha20(ctx->pool, NULL, RNG_POOL_SIZE, &ctx->pool[0], zero_nonce);
  ctx->index = CHACHA20_KEY_SIZE;
}

static size_t min_szt(size_t a, size_t b) { return a < b ? a : b; }

static void rng_read(struct RngCtx *ctx, size_t bufsz, void *buf) {
  unsigned char *bufc = buf;

  while (bufsz > 0) {
    if (ctx->index >= RNG_POOL_SIZE) {
      rng_fill_pool(ctx);
      continue;
    }

    size_t sz = min_szt(bufsz, RNG_POOL_SIZE - ctx->index);
    memcpy(bufc, &ctx->pool[ctx->index], sz);
    bufc += sz;
    bufsz -= sz;
    ctx->index += sz;
  }

  a4r_explicit_bzero(&ctx->pool[CHACHA20_KEY_SIZE], ctx->index - CHACHA20_KEY_SIZE);
}

static int a4r_rng_stir(struct a4r_rng *r) {
  unsigned char seed[RNG_SEED_SIZE];
  if (a4r_getentropy(seed, sizeof seed) == -1)
    return -1;

  rng_init(&r->rng, seed);
  r->is_seeded = 1;
  return 0;
}

void arc4random_buf(void *buf, size_t bufsz) {
  if (a4r_init_global() < 0)
    abort();

  struct a4r_rng *r = a4r_rng_get();
  if (!r->is_seeded) {
    if (a4r_rng_stir(r) < 0)
      abort();
  }

  rng_read(&r->rng, bufsz, buf);

  a4r_rng_put(r);
}

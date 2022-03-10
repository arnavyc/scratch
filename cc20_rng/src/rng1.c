#include <monocypher.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "dbg.h"
#include "rng.h"

void rng_init(RngCtx *ctx, const uint8_t seed[CHACHA20_KEY_SIZE]) {
  memcpy(ctx->pool, seed, CHACHA20_KEY_SIZE);
  ctx->index = RNG_POOL_SIZE;
}

static void rng_fill_pool(RngCtx *ctx) {
  static const uint8_t zero_nonce[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  crypto_chacha20(ctx->pool, NULL, RNG_POOL_SIZE, &ctx->pool[0], zero_nonce);
  ctx->index = CHACHA20_KEY_SIZE;
}

static size_t min_szt(size_t a, size_t b) { return a < b ? a : b; }

void rng_read(RngCtx *ctx, size_t bufsz, void *buf) {
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

  crypto_wipe(&ctx->pool[CHACHA20_KEY_SIZE], ctx->index - CHACHA20_KEY_SIZE);
}

#include <sys/random.h>

#include "arc4random.h"

#include "rng1_private.c"

void a4rand_stir(void) {
  uint8_t seed[CHACHA20_KEY_SIZE];
  if (getentropy(seed, sizeof seed) == -1)
    raise(SIGKILL);

  rng_init(priv__get_rng(), seed);
  reset_fork_status();

  crypto_wipe(seed, sizeof seed);
}

void a4rand_init(void) {
  if (priv__is_initialized())
    return;

  rng_priv_init();
  a4rand_stir();

  priv__set_initialized(true);

  atexit(rng_priv_destroy);
}

void arc4random_buf(void *buf, size_t bufsz) {
  if (dbgb(priv__has_process_forked()))
    a4rand_stir();

  rng_read(priv__get_rng(), bufsz, buf);
}

uint32_t arc4random(void) {
  uint32_t num;
  arc4random_buf(&num, sizeof num);
  return num;
}

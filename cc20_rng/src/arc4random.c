#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/mman.h>
#include <sys/random.h>

#include <monocypher.h>

#include "arc4random.h"

#define RNG_POOL_SIZE 768
#define CHACHA20_KEY_SIZE 32
#define RNG_SEED_SIZE CHACHA20_KEY_SIZE

struct RngCtx {
  uint8_t pool[RNG_POOL_SIZE];
  size_t index;
};

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

  crypto_wipe(&ctx->pool[CHACHA20_KEY_SIZE], ctx->index - CHACHA20_KEY_SIZE);
}

struct a4r_rng {
  struct RngCtx rng;
  unsigned char is_seeded;
};

static struct a4r_rng *a4r_rng_new(void) {
  struct a4r_rng *res = mmap(NULL, sizeof *res, PROT_READ | PROT_WRITE,
                             MAP_ANON | MAP_PRIVATE, -1, 0);

  if (res == MAP_FAILED)
    return NULL;

  madvise(res, sizeof *res, MADV_WIPEONFORK);
  mlock(res, sizeof *res);
  res->is_seeded = 0;

  return res;
}

static struct a4r_globals {
  struct a4r_rng *global_rng;
  pthread_mutex_t lock;
  bool is_initialized;
  pthread_key_t tlocal_key;
} a4r_globals = {.lock = PTHREAD_MUTEX_INITIALIZER, .is_initialized = false};

static void a4r_rng_destroy(void *d);

static int a4r_init_global(void) {
  pthread_mutex_lock(&a4r_globals.lock);

  if (!a4r_globals.is_initialized) {
    a4r_globals.global_rng = a4r_rng_new();
    if (a4r_globals.global_rng == NULL)
      return -1;

    pthread_key_create(&a4r_globals.tlocal_key, a4r_rng_destroy);

    a4r_globals.is_initialized = true;
  }

  pthread_mutex_unlock(&a4r_globals.lock);
  return 0;
}

static void a4r_rng_destroy(void *d) {
  struct a4r_rng *r = d;
  crypto_wipe(r, sizeof *r);

  madvise(r, sizeof *r, MADV_KEEPONFORK);
  munlock(r, sizeof *r);

  munmap(r, sizeof *r);
}

static struct a4r_rng *a4r_rng_get(void) {
  struct a4r_rng *rng = pthread_getspecific(a4r_globals.tlocal_key);
  if (rng != NULL)
    return rng;

  rng = a4r_rng_new();
  if (rng != NULL) {
    pthread_setspecific(a4r_globals.tlocal_key, rng);
    return rng;
  }

  pthread_mutex_lock(&a4r_globals.lock);
  return a4r_globals.global_rng;
}

static void a4r_rng_put(struct a4r_rng *r) {
  if (r == a4r_globals.global_rng)
    pthread_mutex_unlock(&a4r_globals.lock);
}

static int a4r_rng_stir(struct a4r_rng *r) {
  unsigned char seed[RNG_SEED_SIZE];
  if (getentropy(seed, sizeof seed) == -1)
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

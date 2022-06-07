#include <stdbool.h>
#include <stddef.h>

#include <pthread.h>
#include <sys/mman.h>
#include <sys/random.h>

#include "arc4random_private.h"

int a4r_getentropy(void *buffer, size_t length) {
  return getentropy(buffer, length);
}

static struct a4r_globals {
  struct a4r_rng *global_rng;
  pthread_mutex_t lock;
  bool is_initialized;
  pthread_key_t tlocal_key;
} a4r_globals = {.lock = PTHREAD_MUTEX_INITIALIZER, .is_initialized = false};

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

static void a4r_rng_destroy(void *d);

int a4r_init_global(void) {
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

struct a4r_rng *a4r_rng_get(void) {
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

void a4r_rng_put(struct a4r_rng *r) {
  if (r == a4r_globals.global_rng)
    pthread_mutex_unlock(&a4r_globals.lock);
}

static void a4r_rng_destroy(void *d) {
  struct a4r_rng *r = d;
  a4r_explicit_bzero(r, sizeof *r);

  madvise(r, sizeof *r, MADV_KEEPONFORK);
  munlock(r, sizeof *r);

  munmap(r, sizeof *r);
}

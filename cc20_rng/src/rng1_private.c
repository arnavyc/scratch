#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <threads.h>

#include "rng.h"

static unsigned char *is_not_forked = NULL;

static bool priv__has_process_forked(void) {
  return is_not_forked && !*is_not_forked;
}

static void reset_fork_status(void) { *is_not_forked = 1; }

static void init_is_not_forked(void) {
  is_not_forked = mmap(NULL, sizeof is_not_forked[0], PROT_READ | PROT_WRITE,
                       MAP_ANON | MAP_PRIVATE, -1, 0);
  if (is_not_forked == MAP_FAILED)
    raise(SIGKILL);

  madvise(is_not_forked, sizeof is_not_forked[0], MADV_WIPEONFORK);

  reset_fork_status();
}

static once_flag is_not_forked_flag = ONCE_FLAG_INIT;
static void rng_priv_init(void) {
  call_once(&is_not_forked_flag, init_is_not_forked);
}

static void rng_priv_destroy(void) {
  if (is_not_forked)
    munmap(is_not_forked, sizeof is_not_forked[0]);
}

static thread_local struct {
  bool is_initialized;
  RngCtx rng;
} global;

static RngCtx *priv__get_rng(void) { return &global.rng; }
static bool priv__is_initialized(void) { return global.is_initialized; }
static void priv__set_initialized(bool val) { global.is_initialized = val; }

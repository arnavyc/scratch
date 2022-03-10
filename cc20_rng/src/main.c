#include <sys/random.h>

#include "dbg.h"
/* #include "rng.h" */
#include "arc4random.h"

/*
int main(void) {
  uint8_t seed[CHACHA20_KEY_SIZE];
  getentropy(seed, sizeof seed);

  RngCtx rng;
  rng_init(&rng, seed);

  uint8_t buf[120];
  rng_read(&rng, sizeof buf, buf);

  dbgh(buf, sizeof buf);
}*/

int main(void) {
  a4rand_init();

  uint8_t buf[120];
  pid_t f = fork();
  if (f == 0) {
    freopen("b.txt", "w", stderr);
    fprintf(stderr, "In fork\n");
  }

  for (size_t i = 0; i < 2; ++i) {
    arc4random_buf(buf, sizeof buf);
    dbgh(buf, sizeof buf);
  }

  if (f == 0)
    fclose(stderr);
}

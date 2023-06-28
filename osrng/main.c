#include <stdio.h>
#include <stdlib.h>

#include "random.h"

int main(void) {
  FILE *fp = fopen("a.bin", "wb+");
  if (!fp) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  unsigned char a[1024 * 1024];
  int err = getrand(sizeof a, a);
  if (err) {
    fprintf(stderr, "Error in getrand().\n");
  }

  fwrite(a, sizeof a[0], sizeof a / sizeof a[0], fp);

  fclose(fp);
}

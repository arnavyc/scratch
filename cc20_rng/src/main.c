#include "arc4random.h"
#include "dbg.h"

int main(void) {
  unsigned char buf[1024];
  arc4random_buf(buf, sizeof buf);

  dbgh(buf, sizeof buf);

  return EXIT_SUCCESS;
}

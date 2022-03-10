#ifndef AY_ARC4RANDOM_H
#define AY_ARC4RANDOM_H

#include <stddef.h>
#include <stdint.h>

void a4rand_init(void);
// void a4rand_destroy(void);
void arc4random_buf(void *buf, size_t bufsz);
uint32_t arc4random(void);

#endif /* AY_ARC4RANDOM_H */

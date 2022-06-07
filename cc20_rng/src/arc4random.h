#ifndef AY_ARC4RANDOM_H
#define AY_ARC4RANDOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

void arc4random_buf(void *buf, size_t bufsz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AY_ARC4RANDOM_H */

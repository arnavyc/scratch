/* SPDX-License-Identifier: 0BSD */

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __GNUC__
#define AY_GNUC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#else
#define AY_GNUC_VERSION 0
#endif /* __GNUC__ */

#if defined(_MSC_VER)
#include <intrin.h>

#if defined(_M_X64) || defined(_M_AMD64)
#define msvc_umul128(a, b, high) _umul128((a), (b), (high))
#elif defined(_M_ARM64)
static unsigned __int64 msvc_umul128(
    unsigned __int64 a,
    unsigned __int64 b,
    unsigned __int64 *high) {
  *high = __umulh(a, b);
  return (unsigned __int64)a * b;
}
#define msvc_umul128(a, b, high) msvc_umul128((a), (b), (high))
#endif

#endif

int size_t_mul_overflow(size_t *r, size_t a, size_t b) {
#if defined(msvc_umul128)
  unsigned __int64 high;
  *r = msvc_umul128(a, b, &high);
  return high != 0;
#elif AY_GNUC_VERSION >= 50100
  return __builtin_mul_overflow(a, b, r);
#else
  *r = (size_t)a * b;
  return a != 0 && *r / a != b;
#endif
}

void *ay_reallocarray(void *ptr, size_t nmemb, size_t size) {
  size_t total_size = 0;
  if (size_t_mul_overflow(&total_size, nmemb, size)) {
    errno = ENOMEM;
    return NULL;
  }
  return realloc(ptr, total_size);
}

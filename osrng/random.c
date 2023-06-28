#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <bcrypt.h>
#include <windows.h>
#endif /* _WIN32 */

/* Macro for checking GLIBC version. */
#define AY_GLIBC_PREREQ(maj, min)                                              \
  (__GLIBC__ > (maj) || (__GLIBC__ == (maj) && __GLIBC_MINOR__ >= (min)))

#if AY_GLIBC_PREREQ(2, 26)
#include <sys/random.h>
#endif

void c_noop(void *s, ...) {}

#ifndef NDEBUG
#define perror_debug(s) perror(s)
#else
#define perror_debug(s) c_noop(s)
#endif /* NDEBUG */

#include "random.h"

int getrand(size_t buf_size, void *buf) {
#if defined(_WIN32)
  unsigned char *buf_uchar = (unsigned char *)buf;
  NTSTATUS s = BCryptGenRandom(BCRYPT_RNG_ALG_HANDLE, buf_uchar, buf_size, 0);

  return BCRYPT_SUCCESS(s) ? 0 : OSRNG_INTERNAL_ERROR;
#elif AY_GLIBC_PREREQ(2, 26)
  size_t bytes_read = 0;
  unsigned char *buf_uchar = (unsigned char *)buf;

  while (bytes_read < buf_size) {
    ssize_t r = getrandom(&buf_uchar[bytes_read], buf_size - bytes_read, 0);
    if (r == -1)
      return OSRNG_ERROR_RNDSRC_READ;

    bytes_read += (size_t)r;
  }

  return OSRNG_OK;
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
  FILE *fp = fopen("/dev/urandom", "r");
  if (!fp) {
    perror_debug("[DEBUG] fopen error opening /dev/urandom");

    return OSRNG_ERROR_RNDSRC_READ;
  }

  /* Disable buffering. */
  setbuf(fp, NULL);

  size_t bytes_read = fread(buf, 1, buf_size, fp);
  if (bytes_read != buf_size) {
    perror_debug("[DEBUG] fread error /dev/urandom");
    fclose(fp);
    return OSRNG_ERROR_RNDSRC_READ;
  }

  fclose(fp);
  return OSRNG_OK;
#else
  return OSRNG_NO_RNG_AVAILABLE;
#endif
}

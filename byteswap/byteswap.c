#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

uint16_t bswap16(uint16_t num) { return num >> 8 | num << 8; }

uint32_t bswap32(uint32_t num) {
  return ((num >> 24) & 0xff) |      /* move byte 3 to byte 0 */
         ((num << 8) & 0xff0000) |   /* move byte 1 to byte 2 */
         ((num >> 8) & 0xff00) |     /* move byte 2 to byte 1 */
         ((num << 24) & 0xff000000); /* byte 0 to byte 3 */
}

uint64_t bswap64(uint64_t x) {
#ifdef _MSC_VER
  return _byteswap_uint64(x);
#else
  return x >> 56                          /* move byte 7 to byte 0 */
         | ((x >> 40) & 0xff00)           /* move byte 6 to byte 1 */
         | ((x >> 24) & 0xff0000)         /* move byte 5 to byte 2 */
         | ((x >> 8) & 0xff000000)        /* move byte 4 to byte 3 */
         | ((x << 8) & 0xff00000000)      /* move byte 3 to byte 4 */
         | ((x << 24) & 0xff0000000000)   /* move byte 2 to byte 5 */
         | ((x << 40) & 0xff000000000000) /* move byte 1 to byte 6 */
         | x << 56;                       /* move byte 0 to byte 7 */
#endif
}

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* clang-format off */
#define bswap(x) _Generic((x), \
  uint8_t : (x), \
  uint16_t: bswap16(x), \
  uint32_t: bswap32(x), \
  uint64_t: bswap64(x), \
  int8_t  : (x), \
  int16_t : (int16_t)bswap16((uint16_t)(x)), \
  int32_t : (int32_t)bswap32((uint32_t)(x)), \
  int64_t : (int64_t)bswap64((uint64_t)(x)) \
  )
/* clang-format on */
#endif

uint16_t load_leu16(const unsigned char a[static sizeof(uint16_t)]) {
  return ((uint16_t)a[1] << 8) | (uint8_t)a[0];
}

uint32_t load_leu32(const unsigned char a[static sizeof(uint32_t)]) {
  return (uint32_t)a[3] << 24 | (uint32_t)a[2] << 16 | (uint32_t)a[1] << 8 |
         (uint32_t)a[0];
}

uint64_t load_leu64(const unsigned char a[static sizeof(uint64_t)]) {
  return (uint64_t)a[7] << 56 | (uint64_t)a[6] << 48 | (uint64_t)a[5] << 40 |
         (uint64_t)a[4] << 32 | (uint64_t)a[3] << 24 | (uint64_t)a[2] << 16 |
         (uint64_t)a[1] << 8 | (uint64_t)a[0];
}

uint16_t load_beu16(const unsigned char a[static sizeof(uint16_t)]) {
  return ((uint16_t)a[0] << 8) | (uint8_t)a[1];
}

uint32_t load_beu32(const unsigned char a[static sizeof(uint32_t)]) {
  return (uint32_t)a[0] << 24 | (uint32_t)a[1] << 16 | (uint32_t)a[2] << 8 |
         (uint32_t)a[3];
}

uint64_t load_beu64(const unsigned char a[static sizeof(uint64_t)]) {
  return (uint64_t)a[0] << 56 | (uint64_t)a[1] << 48 | (uint64_t)a[2] << 40 |
         (uint64_t)a[3] << 32 | (uint64_t)a[4] << 24 | (uint64_t)a[5] << 16 |
         (uint64_t)a[6] << 8 | (uint64_t)a[7];
}

int main(void) {}

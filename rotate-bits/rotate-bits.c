/* SPDX-License-Identifier: 0BSD */
#include <stdint.h>
#include <limits.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#if !defined(_MSC_VER)
#  define AY_ROT_MSVC_PREREQ(major,minor) (0)
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#  define AY_ROT_MSVC_PREREQ(major,minor) (_MSC_FULL_VER >= ((major * 1000000) + (minor * 10000)))
#elif defined(_MSC_VER) && (_MSC_VER >= 1200)
#  define AY_ROT_MSVC_PREREQ(major,minor) (_MSC_FULL_VER >= ((major * 100000) + (minor * 1000)))
#else
#  define AY_ROT_MSVC_PREREQ(major,minor) (_MSC_VER >= ((major * 100) + (minor)))
#endif

#define generic_mask(n) (CHAR_BIT * sizeof (n) - 1)
#define generic_negate(n) (-(n))
#define generic_rotl(n, c) ((n) << ((c) & generic_mask(n)) | ((n) >> (generic_negate(c) & generic_mask(n))))
#define generic_rotr(n, c) ((n) >> ((c) & generic_mask(n)) | ((n) << (generic_negate(c) & generic_mask(n))))

uint8_t rotl8(uint8_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(14, 0)
  return _rotl8(n, c);
#else
  return generic_rotl(n, c);
#endif
}

uint16_t rotl16(uint16_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(14, 0)
  return _rotl16(n, c);
#else
  return generic_rotl(n, c);
#endif
}

uint32_t rotl32 (uint32_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(13, 10)
  return _rotl(n, c);
#else
  return generic_rotl(n, c);
#endif
}

uint64_t rotl64 (uint64_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(13, 10)
  return _rotl64(n, c);
#else
  return generic_rotl(n, c);
#endif
}

uint8_t rotr8(uint8_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(14, 0)
  return _rotr8(n, c);
#else
  return generic_rotl(n, c);
#endif
}

uint16_t rotr16(uint16_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(14, 0)
  return _rotr16(n, c);
#else
  return generic_rotr(n, c);
#endif
}

uint32_t rotr32(uint32_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(13, 10)
  return _rotr(n, c);
#else
  return generic_rotr(n, c);
#endif
}

uint64_t rotr64(uint64_t n, unsigned char c) {
#if AY_ROT_MSVC_PREREQ(13, 0)
  return _rotr64(n, c);
#else
  return generic_rotr(n, c);
#endif
}
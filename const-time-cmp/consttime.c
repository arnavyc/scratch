/* SPDX-License-Identifier: 0BSD */
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>

#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define AY_CT_GNUC_PREREQ(maj, min)                                            \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define AY_CT_GNUC_PREREQ(maj, min) (0)
#endif

#if defined(_MSC_VER)
#define AY_CT_MSVC_PREREQ(major, minor) (_MSC_VER >= ((major * 100) + (minor)))
#else
#define AY_CT_MSVC_PREREQ(major, minor) (0)
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) &&              \
    !defined(__cplusplus)
#define AY_CT_RESTRICT restrict
#elif AY_CT_GNUC_PREREQ(3, 1) || AY_CT_MSVC_PREREQ(14, 0)
#define AY_CT_RESTRICT __restrict
#else
#define AY_CT_RESTRICT
#endif

/**
 * @brief Returns 0xffffffff if a == 1, else 0 if a == 0
 *
 * @param a number having value either 0 or 1
 * @returns 0xffffffff if a == 1, 0 if a == 0, undefined for all other values
 */
uint32_t ct_mask_u32(uint32_t a) { return (uint32_t)(-(int32_t)a); }

/**
 * @brief Returns a or b depending on whether cond == 1 or 0
 *
 * @param cond number containing either 0 or 1
 * @param a number to return if cond == 1
 * @param b number to return if cond == 0
 * @returns a if cond == 1, b if cond == 0, undefined for all other values
 */
uint32_t ct_select_u32(unsigned char cond, uint32_t a, uint32_t b) {
  uint32_t result = 0;

#if defined(__GNUC__) && defined(__x86_64__) && !defined(AY_CT_FORCE_PORTABLE)
  result = b;

  __asm__ __volatile__(
      "test %[cond], %[cond]\n\tcmovne {%[a], %[result] | %[result], %[a]}"
      : [result] "+r"(result)
      : [cond] "r"(cond), [a] "rm"(a)
      : "cc");
#else
  uint32_t mask = ct_mask_u32(cond);
  result = (a & mask) | (b & ~mask);
#endif

  return result;
}

/**
 * @brief Negates boolean `a` stored in unsigned char that is either 1 or 0.
 *
 * @param a 'boolean' to negate
 * @returns 0 if a == 1, 1 if a == 0, undefined for all other values.
 */
unsigned char ct_negate(unsigned char a) { return 1 ^ a; }

/**
 * @brief Check whether a != 0 or not.
 *
 * @param a number to be checked
 * @returns `true` (`1`) if a != 0, false (0) otherwise.
 */
unsigned char ct_is_non_zero_u32(uint32_t a) {
  return (unsigned char)((a | -a) >> (sizeof(uint32_t) * CHAR_BIT - 1));
}

/**
 * @brief Check whether a == 0 or not.
 *
 * @param a number to be checked
 * @returns true (1) if a == 0, false (0) otherwise.
 */
unsigned char ct_is_zero_u32(uint32_t a) {
  return ct_negate(ct_is_non_zero_u32(a));
}

/**
 * @brief Checks if lhs != rhs in constant time.
 *
 * @param lhs number before `!=`
 * @param rhs number after `!=`
 * @returns true (1) if lhs != rhs, false (0) otherwise.
 */
unsigned char ct_is_neq_u32(uint32_t lhs, uint32_t rhs) {
  return ct_is_non_zero_u32(lhs ^ rhs);
}

/**
 * @brief Checks if lhs == rhs in constant time.
 *
 * @param lhs number before `==`
 * @param rhs number after `==`
 * @returns true (1) if lhs == rhs, false (0) otherwise.
 */
unsigned char ct_is_eq_u32(uint32_t lhs, uint32_t rhs) {
  return ct_negate(ct_is_neq_u32(lhs, rhs));
}

/**
 * @brief Checks if lhs <= rhs in constant time.
 *
 * @param lhs number before `<=`
 * @param rhs number after `<=`
 * @returns true (1) if lhs <= rhs, false (0) otherwise.
 */
unsigned char ct_is_le_u32(uint32_t lhs, uint32_t rhs) {
  const unsigned char shift_value = sizeof lhs * CHAR_BIT - 1;
  return (unsigned char)((lhs - rhs - 1) >> shift_value) & 1;
}

/**
 * @brief Checks if lhs >= rhs in constant time.
 *
 * @param lhs number before `>=`
 * @param rhs number after `>=`
 * @returns true (1) if lhs >= rhs, false (0) otherwise.
 */
unsigned char ct_is_ge_u32(uint32_t lhs, uint32_t rhs) {
  return ct_is_le_u32(rhs, lhs);
}

/**
 * @brief Checks if lhs > rhs in constant time.
 *
 * @param lhs number before `>`
 * @param rhs number after `>`
 * @returns true (1) if lhs > rhs, false (0) otherwise.
 */
unsigned char ct_is_gt_u32(uint32_t lhs, uint32_t rhs) {
  return ct_negate(ct_is_le_u32(lhs, rhs));
}

/**
 * @brief Checks if lhs < rhs in constant time.
 *
 * @param lhs number before `<`
 * @param rhs number after `<`
 * @returns true (1) if lhs < rhs, false (0) otherwise.
 */
unsigned char ct_is_lt_u32(uint32_t lhs, uint32_t rhs) {
  return ct_is_gt_u32(rhs, lhs);
}

/**
 * @brief Lexicographically compare the first len bytes (each interpreted as an
 * unsigned char) pointed to by a and b to check whether the first len bytes are
 * equal or not.
 *
 * @param a first byte sequence
 * @param b second byte sequence
 * @param len number of bytes to be compared
 *
 * @returns 0 or not zero if the byte sequence pointed to by a compares equal to
 * or not equal to (respectively) the byte sequence pointed to by b.
 */
int ct_timingsafe_bcmp(const void *a, const void *b, size_t len) {
  const volatile unsigned char *p1 = (const volatile unsigned char *)a;
  const volatile unsigned char *p2 = (const volatile unsigned char *)b;
  int result = 0;

  for (size_t i = 0; i < len; ++i)
    result |= p1[i] ^ p2[i];

  return result;
}

/**
 * @brief Lexicographically compare the first len bytes (each interpreted as an
 * unsigned char) pointed to by a and b to check whether the byte sequence
 * pointed to a compares less than, equal to or greater than the byte sequence
 * pointed to b.
 *
 * @param a first byte sequence
 * @param b second byte sequence
 * @param len number of bytes to be compared
 *
 * @returns a negative value, 0, or positive value if the byte sequence pointed
 * to by a compares less than, equal to, or greater than (respectively) the byte
 * sequence pointed to by b.
 */
int ct_timingsafe_memcmp(const void *a, const void *b, size_t len) {
  const volatile unsigned char *p1 = (const volatile unsigned char *)a;
  const volatile unsigned char *p2 = (const volatile unsigned char *)b;
  int result = 0;
  int processing_done = 0;

  for (size_t i = 0; i < len; ++i) {
    unsigned char gt = ct_is_gt_u32(p1[i], p2[i]);
    unsigned char lt = ct_is_lt_u32(p1[i], p2[i]);
    int cmp = gt - lt;

    result |= cmp & ~processing_done;
    processing_done |= lt | gt;
  }

  return result;
}

int main(void) {}
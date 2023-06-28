#ifndef U128_H
#define U128_H

#include <builtin/builtin.h>
#include <endian/endian.h>
#include <stdint.h>

#ifndef INLINE
#if __GNUC__ && !__GNUC_STDC_INLINE__
#define INLINE extern inline
#else
#define INLINE inline
#endif
#endif

#if PSNIP_ENDIAN_ORDER == PSNIP_ENDIAN_BIG
#define U128_UPPER_IDX 0
#define U128_LOWER_IDX 1
#elif PSNIP_ENDIAN_ORDER == PSNIP_ENDIAN_LITTLE
#define U128_LOWER_IDX 0
#define U128_UPPER_IDX 1
#else
#error Could not determine endianness of system
#endif

typedef struct u128 {
  uint64_t halves[2];
} u128;

INLINE uint64_t u128_get_lower(u128 result) {
  return result.halves[U128_LOWER_IDX];
}

INLINE uint64_t u128_get_upper(u128 result) {
  return result.halves[U128_UPPER_IDX];
}

INLINE void u128_set_lower(u128 *result, uint64_t lower) {
  result->halves[U128_LOWER_IDX] = lower;
}

INLINE void u128_set_upper(u128 *result, uint64_t upper) {
  result->halves[U128_UPPER_IDX] = upper;
}

INLINE u128 u128_init_from_u64(uint64_t upper, uint64_t lower) {
  u128 result;
  u128_set_upper(&result, upper);
  u128_set_lower(&result, lower);

  return result;
}

INLINE void u128_addto(u128 *result, u128 lhs, u128 rhs) {
  uint64_t lower = u128_get_lower(lhs) + u128_get_lower(rhs);
  uint64_t carry = lower < u128_get_lower(lhs) ? 1 : 0;
  uint64_t upper = u128_get_upper(lhs) + u128_get_upper(rhs) + carry;

  u128_set_lower(result, lower);
  u128_set_upper(result, upper);
}

INLINE u128 u128_add(u128 lhs, u128 rhs) {
  u128 result;
  u128_addto(&result, lhs, rhs);

  return result;
}

INLINE void u128_subtractfrom(u128 *result, u128 lhs, u128 rhs) {
  uint64_t lower = u128_get_lower(lhs) - u128_get_lower(rhs);
  uint64_t borrow = lower > u128_get_lower(lhs) ? 1 : 0;
  uint64_t upper = u128_get_upper(lhs) - u128_get_upper(rhs) - borrow;

  u128_set_lower(result, lower);
  u128_set_upper(result, upper);
}

INLINE u128 u128_subtract(u128 lhs, u128 rhs) {
  u128 result;
  u128_subtractfrom(&result, lhs, rhs);

  return result;
}

INLINE void u128_increment(u128 *number) {
  u128 u128_1 = u128_init_from_u64(0, 1);
  *number = u128_add(*number, u128_1);
}

INLINE u128 u128_multiply(u128 a, u128 b) {
  uint64_t top[4] = {u128_get_upper(a) >> (sizeof(uint64_t) * CHAR_BIT / 2),
                     u128_get_upper(a) & 0xffffffff,
                     u128_get_lower(a) >> (sizeof(uint64_t) * CHAR_BIT / 2),
                     u128_get_lower(a) & 0xffffffff};
  uint64_t bottom[4] = {u128_get_upper(b) >> (sizeof(uint64_t) * CHAR_BIT / 2),
                        u128_get_upper(b) & 0xffffffff,
                        u128_get_lower(b) >> (sizeof(uint64_t) * CHAR_BIT / 2),
                        u128_get_lower(b) & 0xffffffff};
  uint64_t products[4][4];

  for (size_t y = 5; y-- > 0;) {
    for (size_t x = 4; x-- > 0;) {
      products[3 - x][y] = top[x] * bottom[x];
    }
  }

  /* Add first row */
  uint64_t fourth_col = (products[0][3] & 0xffffffff);
  uint64_t third_col = (products[0][2] & 0xffffffff) + (products[0][3] >> 32);
  uint64_t second_col = (products[0][1] & 0xffffffff) + (products[0][2] >> 32);
  uint64_t first_col = (products[0][0] & 0xffffffff) + (products[0][1] >> 32);

  /* Add second row */
  third_col += (products[1][3] & 0xffffffff);
  second_col += (products[1][2] & 0xffffffff) + (products[1][3] >> 32);
  first_col += (products[1][1] & 0xffffffff) + (products[1][2] >> 32);

  /* Add third row */
  second_col += (products[2][3] & 0xffffffff);
  first_col += (products[2][2] & 0xffffffff) + (products[2][3] >> 32);

  /* Add fourth row */
  first_col += (products[3][3] & 0xffffffff);

  /* move carry to next digit */
  third_col += fourth_col >> 32;
  second_col += third_col >> 32;
  first_col += second_col >> 32;

  /* remove carry from current digit */
  fourth_col &= 0xffffffff;
  third_col &= 0xffffffff;
  second_col &= 0xffffffff;
  first_col &= 0xffffffff;

  return u128_init_from_u64((first_col << 32) | second_col,
                            (third_col << 32) | fourth_col);
}

INLINE u128 u128_xor(u128 a, u128 b) {
  return u128_init_from_u64(u128_get_upper(a) ^ u128_get_upper(b),
                            u128_get_lower(a) ^ u128_get_lower(b));
}

INLINE u128 u128_or(u128 a, u128 b) {
  return u128_init_from_u64(u128_get_upper(a) | u128_get_upper(b),
                            u128_get_lower(a) | u128_get_lower(b));
}

INLINE u128 u128_and(u128 a, u128 b) {
  return u128_init_from_u64(u128_get_upper(a) & u128_get_upper(b),
                            u128_get_lower(a) & u128_get_lower(b));
}

INLINE u128 u128_not(u128 a) {
  return u128_init_from_u64(~u128_get_upper(a), ~u128_get_lower(a));
}

INLINE int8_t u128_compare(u128 lhs, u128 rhs) {
  if (u128_get_upper(lhs) == u128_get_upper(rhs)) {
    if (u128_get_lower(lhs) == u128_get_lower(rhs)) {
      return 0;
    } else {
      return u128_get_lower(lhs) < u128_get_lower(rhs) ? -1 : 1;
    }
  }

  return u128_get_upper(lhs) < u128_get_upper(rhs) ? -1 : 1;
}

#define U64_BIT (CHAR_BIT * sizeof(uint64_t))

INLINE u128 u128_shift_left(u128 number, uint8_t shift) {
  if (shift == 64) {
    return u128_init_from_u64(u128_get_lower(number), 0);
  } else if (shift == 0) {
    return number;
  } else if (shift < 64) {
    return u128_init_from_u64((u128_get_upper(number) << shift) +
                                  (u128_get_lower(number) >> (U64_BIT - shift)),
                              u128_get_lower(number) << shift);
  } else if (shift < 128) {
    return u128_init_from_u64(u128_get_lower(number) << (shift - U64_BIT), 0);
  }

  /* All other cases (including shift >= 128) handled here. */
  return u128_init_from_u64(0, 0);
}

INLINE u128 u128_shift_right(u128 number, uint8_t shift) {
  if (shift == 64) {
    return u128_init_from_u64(0, u128_get_upper(number));
  } else if (shift == 0) {
    return number;
  } else if (shift < 64) {
    return u128_init_from_u64(u128_get_upper(number) >> shift,
                              (u128_get_upper(number) << (64 - U64_BIT)) +
                                  (u128_get_lower(number) >> shift));
  } else if (shift < 128) {
    return u128_init_from_u64(
      0,
      u128_get_upper(number) >> (shift - U64_BIT)
    );
  }

  /* All other cases (including shift >= 128) handled here. */
  return u128_init_from_u64(0, 0);
}

INLINE uint8_t u128_clz(u128 a) {
  return u128_get_upper(a) == 0 ? 64 + psnip_builtin_clz64(u128_get_lower(a))
                                : psnip_builtin_clz64(u128_get_upper(a));
}

//uint8_t clz_u128(unsigned __int128 a) {
//  return (a >> 64) == 0
//             ? 64 + psnip_builtin_clz64((uint64_t)(a & 0xffffffffffffffff))
//             : __builtin_clzll((uint64_t)(a >> 64));
//}

//unsigned __int128 div_u128(unsigned __int128 dividend,
//                           unsigned __int128 divisor,
//                           unsigned __int128 *remainder) {
//  if (divisor > dividend) {
//    if (remainder) {
//      *remainder = dividend;
//    }
//    return 0;
//  }
//
//  uint8_t shift = clz_u128(divisor) - clz_u128(dividend);
//  divisor <<= shift;
//  unsigned __int128 quotient = 0;
//
//  for (uint8_t n = shift + 1; n-- > 0;) {
//    quotient <<= 1;
//
//    if (dividend >= divisor) {
//      dividend -= divisor;
//      quotient |= 1;
//    }
//    divisor >>= 1;
//  }
//
//  if (remainder) {
//    *remainder = dividend;
//  }
//
//  return quotient;
//}

INLINE u128 u128_divide(u128 dividend,
                             u128 divisor,
                             u128 *remainder) {
  if (u128_compare(divisor, dividend) > 0) {
    if (remainder) {
      *remainder = dividend;
    }
    return u128_init_from_u64(0, 0);
  }

  uint8_t shift = u128_clz(divisor) - u128_clz(dividend);
  divisor = u128_shift_left(divisor, shift);
  u128 quotient = u128_init_from_u64(0, 0);

  for (uint8_t n = shift + 1; n-- > 0;) {
    quotient = u128_shift_left(quotient, 1);

    if (u128_compare(dividend, divisor) >= 0) {
      dividend = u128_subtract(dividend, divisor);
      quotient = u128_or(quotient, u128_init_from_u64(0, 1));
    }
    divisor = u128_shift_right(divisor, 1);
  }

  if (remainder) {
    *remainder = dividend;
  }

  return quotient;
}

#endif /* U128_H */

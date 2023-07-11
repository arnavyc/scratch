#ifndef AY_UUID_H
#define AY_UUID_H

#include <stdint.h>

#define AY_UUID_NUM_BYTES 16
#define AY_UUID_NUM_CHARS 37
#define AY_UUID_NUM_CHARS_WITHOUT_NULL (AY_UUID_NUM_CHARS - 1)

#define AY_UUID_INFO_OK 0
#define AY_UUID_ERR_CSPRNG -1
#define AY_UUID_ERR_TIMESTAMP -2

int ay_uuid_generate_v4(uint8_t out_uuid[AY_UUID_NUM_BYTES]);
int ay_uuid_generate_v7(uint8_t out_uuid[AY_UUID_NUM_BYTES]);
void ay_uuid_to_string(const uint8_t uuid[AY_UUID_NUM_BYTES], char *out_str);

#ifdef AY_UUID_IMPLEMENTATION
#include <assert.h>
#include <time.h>

#include <sys/random.h>

static void define_version_and_variant_rfc(uint8_t uuid[AY_UUID_NUM_BYTES],
                                           unsigned char version) {
  static const unsigned char max_version = (unsigned char)(1 << 4) - 1;
  assert(version <= max_version);

  /* In 7th byte replace higher nibble with 0x0100 (0x4) */
  uuid[6] = (uuid[6] & 0x0f) | (version << 4);

  /* In 9th byte replace highest 2 bits with 0b10 */
  uuid[8] = (uuid[8] & 0x3f) | 0x80;
}

int ay_uuid_generate_v4(uint8_t out_uuid[AY_UUID_NUM_BYTES]) {
  if (getrandom(out_uuid, AY_UUID_NUM_BYTES, 0) < 0)
    return AY_UUID_ERR_CSPRNG;

  define_version_and_variant_rfc(out_uuid, 4);
  return AY_UUID_INFO_OK;
}

int ay_uuid_generate_v7(uint8_t out_uuid[AY_UUID_NUM_BYTES]) {
  struct timespec current_ts = {0};
  if (!timespec_get(&current_ts, TIME_UTC))
    return AY_UUID_ERR_TIMESTAMP;

  uint64_t current_timestamp = (uint64_t)current_ts.tv_sec * 1000 +
                               (uint64_t)current_ts.tv_nsec / 1000000;
  out_uuid[0] = current_timestamp >> 40;
  out_uuid[1] = current_timestamp >> 32;
  out_uuid[2] = current_timestamp >> 24;
  out_uuid[3] = current_timestamp >> 16;
  out_uuid[4] = current_timestamp >> 8;
  out_uuid[5] = current_timestamp;

  if (getrandom(&out_uuid[6], AY_UUID_NUM_BYTES - 6, 0) < 0)
    return AY_UUID_ERR_CSPRNG;

  define_version_and_variant_rfc(out_uuid, 7);
  return AY_UUID_INFO_OK;
}

void ay_uuid_to_string(const uint8_t uuid[AY_UUID_NUM_BYTES], char *out_str) {
  static const char DIGITS[] = "0123456789abcdef";

  for (int i = 0, j = 0; i < 16; i++) {
    uint_fast8_t b = uuid[i];
    out_str[j++] = DIGITS[b >> 4];
    out_str[j++] = DIGITS[b & 0x0f];

    if (i == 3 || i == 5 || i == 7 || i == 9)
      out_str[j++] = '-';
  }
  out_str[AY_UUID_NUM_CHARS - 1] = '\0';
}

#endif /* AY_UUID_IMPLEMENTATION */

#endif /* AY_UUID_H */

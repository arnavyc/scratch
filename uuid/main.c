#include <stdio.h>

#include "uuid.h"

int main(int argc, char *argv[]) {
  uint8_t uuid[AY_UUID_NUM_BYTES];
  char uuid_str[AY_UUID_NUM_CHARS];

  ay_uuid_generate_v4(uuid);
  ay_uuid_to_string(uuid, uuid_str);
  printf("UUIDv4: %s\n", uuid_str);

  ay_uuid_generate_v7(uuid);
  ay_uuid_to_string(uuid, uuid_str);
  printf("UUIDv7: %s\n", uuid_str);
}

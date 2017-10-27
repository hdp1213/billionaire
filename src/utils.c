#include "utils.h"

#include <stdio.h>

char*
hash_addr(const char* addr, size_t length)
{
  unsigned int hash = 5381;
  char* hash_str;

  /* Check that either i exceeds length, or that the current
     character is the terminating \0 character */
  for (size_t i = 0; *addr != '\0' || i < length; ++addr, ++i) {
    hash = ((hash << 5) + hash) + (*addr);
  }

  hash_str = calloc(HASH_LENGTH, sizeof(char));
  snprintf(hash_str, HASH_LENGTH, "%x", hash);

  return hash_str;
}

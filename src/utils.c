#include "utils.h"

#include <err.h>
#include <stdio.h>

char*
hash_addr(const char* addr, size_t length)
{
  unsigned int hash = 5381;
  char* hash_str = NULL;

  /* Check that either i exceeds length, or that the current
     character is the terminating \0 character */
  for (size_t i = 0; *addr != '\0' || i < length; ++addr, ++i) {
    hash = ((hash << 5) + hash) + (*addr);
  }

  hash_str = calloc(HASH_LENGTH, sizeof(char));

  if (hash_str == NULL) {
    err(1, "malloc failed");
  }

  snprintf(hash_str, HASH_LENGTH, "%x", hash);

  return hash_str;
}

const char*
JSON_to_str(json_object* json_obj, size_t* str_len)
{
  return json_object_to_json_string_length(json_obj,
                                           JSON_C_TO_STRING_PLAIN,
                                           str_len);
}

json_object*
get_JSON_value(json_object* json_obj, const char* key)
{
  json_bool has_field;
  json_object* json_value = NULL;

  has_field = json_object_object_get_ex(json_obj,
                                        key,
                                        &json_value);

  if (has_field == FALSE || json_value == NULL) {
    err(1, "failed to extract value corresponding to '%s'", key);
  }

  return json_value;
}

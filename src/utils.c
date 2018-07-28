#include "utils.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <xxhash.h>

#include "command_error.h"

uint32_t
hash_xxhash(const char* key)
{
  unsigned int seed = 0;
  size_t length = strlen(key);

  XXH32_hash_t hash = XXH32((const void*) key, length, seed);

  return (uint32_t) hash;
}

uint32_t
mix(uint32_t a, uint32_t b, uint32_t c)
{
  a=a-b;  a=a-c;  a=a^(c >> 13);
  b=b-c;  b=b-a;  b=b^(a << 8);
  c=c-a;  c=c-b;  c=c^(b >> 13);
  a=a-b;  a=a-c;  a=a^(c >> 12);
  b=b-c;  b=b-a;  b=b^(a << 16);
  c=c-a;  c=c-b;  c=c^(b >> 5);
  a=a-b;  a=a-c;  a=a^(c >> 3);
  b=b-c;  b=b-a;  b=b^(a << 10);
  c=c-a;  c=c-b;  c=c^(b >> 15);
  return c;
}

char*
hash_addr(const char* addr)
{
  uint32_t hash = hash_xxhash(addr);

  char* hash_str = calloc(HASH_LENGTH, sizeof(char));

  if (hash_str == NULL) {
    err(1, "hash_str malloc failed");
  }

  snprintf(hash_str, HASH_LENGTH, "%08x", hash);

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
str_to_JSON(const char* json_str, size_t json_str_len)
{
  enum json_tokener_error jerr;

  json_tokener* tok = json_tokener_new();

  /* New object is initialised, must be appropriately handled later on */
  json_object* parse_obj = json_tokener_parse_ex(tok, json_str,
                                                 (int) json_str_len);

  jerr = json_tokener_get_error(tok);
  json_tokener_free(tok);

  /* Malformed JSON */
  if (jerr != json_tokener_success) {
    cmd_errno = (int) jerr;
    return NULL;
  }

  return parse_obj;
}

json_object*
get_JSON_value(json_object* json_obj, const char* key)
{
  json_bool has_field;
  json_object* json_value = NULL;

  has_field = json_object_object_get_ex(json_obj,
                                        key,
                                        &json_value);

  if (!has_field || json_value == NULL) {
    cmd_errno = (int) EJSONVAL;
    return NULL;
  }

  return json_value;
}

#include "utils.h"
#include "command_error.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <xxhash.h>

uint32_t
hash_djb2(const char* key)
{
  const uint8_t* data = (const uint8_t*) key;

  uint32_t hash = 5381;
  int c;

  while ((c = *data++)) {
    hash = ((hash << 5) + hash) ^ c;
  }

  return hash;
}

uint32_t
hash_xxhash(const char* key)
{
  unsigned int seed = 0;
  size_t length = strlen(key);

  XXH32_hash_t hash = XXH32((const void*) key, length, seed);

  return (uint32_t) hash;
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

size_t
get_next_highest_power_of_two(size_t num)
{
  size_t result = num;

  result--;
  result |= result >> 1;
  result |= result >> 2;
  result |= result >> 4;
  result |= result >> 8;
  result |= result >> 16;
  result++;

  return result;
}

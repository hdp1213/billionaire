#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h> /* uint32_t */
#include <stdlib.h>

#include <json-c/json.h>

#define HASH_LENGTH 9

#define JSON_ARRAY_FOREACH(obj, json_array)                     \
  size_t i, array_len = json_object_array_length((json_array)); \
  json_object* obj;                                             \
  for (i = 0, obj = json_object_array_get_idx((json_array), i); \
       i < array_len;                                           \
       ++i, obj = json_object_array_get_idx((json_array), i))

/**
 * Hashes a string key using the xxHash algorithm.
 */
uint32_t hash_xxhash(const char* key);

/**
 * Mixes three 32-bit integers.
 *
 * From http://burtleburtle.net/bob/hash/doobs.html, code in public
 * domain.
 */
uint32_t mix(uint32_t a, uint32_t b, uint32_t c);

/**
 * Return a 8-digit hex string of a 32-bit hash based on the client
 * address.
 *
 * Used to uniquely identify clients based on their address.
 */
char* hash_addr(const char* addr);

/**
 * Convert a JSON object to a C string.
 *
 * Also return the length of the resulting C string to str_len.
 */
const char* JSON_to_str(json_object* json_obj, size_t* str_len);

/**
 * Convert a C string to a JSON object.
 *
 * Sets cmd_errno to jerr, on failure returns NULL.
 */
json_object* str_to_JSON(const char* json_str, size_t json_str_len);

/**
 * Extract a value from a JSON object given its key.
 *
 * Sets cmd_errno to EJSONVAL, on failure returns NULL.
 */
json_object* get_JSON_value(json_object* json_obj, const char* key);

#endif

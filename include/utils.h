#ifndef _UTILS_H_
#define _UTILS_H_

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
 * Hashes a string key using the DJB2 hash function.
 *
 * Taken from http://www.cse.yorku.ca/~oz/hash.html.
 */
unsigned long hash_djb2(const char* key);

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

/**
 * Get the next highest power of two of given 32-bit num.
 *
 * Only works for 0 <= num <= UINT_MAX/2 + 1, otherwise an overflow
 * condition is reached.
 *
 * Method taken from:
 * https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
size_t get_next_highest_power_of_two(size_t num);

#endif

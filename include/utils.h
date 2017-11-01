#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>

#include <json-c/json.h>

#define HASH_LENGTH 9

/**
 * Return a 8-digit hex string of a 32-bit hash based on the client
 * address.
 *
 * Used to uniquely identify clients based on their address.
 */
char* hash_addr(const char* addr, size_t length);

/**
 * Convert a JSON object to a C string.
 *
 * Also return the length of the resulting C string to str_len.
 */
const char* JSON_to_str(json_object* json_obj, size_t* str_len);

/**
 * Extract a value from a JSON object given its key
 */
json_object* get_JSON_value(json_object* json_obj, const char* key);

#endif

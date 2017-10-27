#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>

#define HASH_LENGTH 9

/**
 * Return a 8-digit hex string of a 32-bit hash based on the client
 * address.
 *
 * Used to uniquely identify clients based on their address.
 */
char* hash_addr(const char* addr, size_t length);

#endif

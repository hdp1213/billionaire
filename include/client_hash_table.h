#ifndef _CLIENT_HASH_TABLE_H_
#define _CLIENT_HASH_TABLE_H_

#include "client.h"

typedef struct client_hash_table client_hash_table;

/**
 * A struct to use as a hash table for constant-time client access.
 *
 * It uses the xxHash algorithm to allocate buckets.
 *
 * It assumes that identical keys correspond to identical clients. This
 * may not be the case when the xxHash has a collision in generating two
 * client IDs. Be warned.
 *
 * This hash table has a fixed size.
 */
struct client_hash_table {
  /* The number of buckets in the table */
  size_t size;

  /* The buckets containing client pointers. */
  client** buckets;
};

/**
 * The static hash table containing all hashed clients.
 */
static client_hash_table* hashed_clients;

/**
 * Create an empty client hash table with defined size.
 */
client_hash_table* client_hash_table_new(size_t size);

/**
 * Insert a client into the hash table.
 *
 * The client's ID is used as the key, and the value is the client
 * itself.
 */
void put_client(client_hash_table* table_obj, client* client_obj);

/**
 * Return a client struct in constant time given its unique id.
 *
 * This method does not remove the client from the table.
 */
client* get_client(const client_hash_table* table_obj, const char* client_id);

/**
 * Remove a client from the table.
 *
 * This method does not deallocate memory associated with the client.
 */
void del_client(client_hash_table* table_obj, const char* client_id);

/**
 * Get the bucket index by computing a modulo on the hash.
 */
size_t get_bucket_idx(const client_hash_table* table_obj, const char* client_id);

/**
 * Free memory allocated to the hash table.
 *
 * The hash table exists for the duration of a game to keep track of
 * all players.
 *
 * The hash table must be already cleaned of all clients before being
 * freed, as otherwise freeing the table will free client objects that
 * may still have pointers elsewhere in the code.
 */
void free_client_hash_table(client_hash_table* table_obj);

#endif

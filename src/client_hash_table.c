#include "client_hash_table.h"

#include <err.h>
#include <string.h>

#include "utils.h"

client_hash_table*
client_hash_table_new(size_t size)
{
  client_hash_table* new_table = malloc(sizeof(client_hash_table));

  if (new_table == NULL) {
    err(1, "new_table malloc failed");
  }

  new_table->size = size;
  new_table->buckets = malloc(new_table->size*sizeof(client*));

  if (new_table->buckets == NULL) {
    err(1, "new_table->buckets malloc failed");
  }

  for (size_t i = 0; i < new_table->size; ++i) {
    new_table->buckets[i] = NULL;
  }

  return new_table;
}

void
put_client(client_hash_table* table_obj, client* client_obj)
{
  size_t bucket_idx = get_bucket_idx(table_obj, client_obj->id);

  client* current = table_obj->buckets[bucket_idx];

  if (current == NULL) {
    table_obj->buckets[bucket_idx] = client_obj;
    return;
  }

  /* Resolve hash collision through separate chaining */
  client* next = NULL;

  while (current != NULL) {
    next = current->next_hash;

    /* Client already exists! */
    if (client_eq(current, client_obj)) {
      /* TODO: set error */
      return;
    }

    if (next == NULL) {
      current->next_hash = client_obj;
      return;
    }

    current = next;
  }

  /* Control will never reach this part of the function */
}

client*
get_client(const client_hash_table* table_obj, const char* client_id)
{
  size_t bucket_idx = get_bucket_idx(table_obj, client_id);

  client* current = table_obj->buckets[bucket_idx];
  client* next = NULL;

  while (current != NULL) {
    next = current->next_hash;

    if (memcmp(current->id, client_id, HASH_LENGTH) == 0) {
      return current;
    }

    current = next;
  }

  return NULL;
}

void
del_client(client_hash_table* table_obj, const char* client_id)
{
  size_t bucket_idx = get_bucket_idx(table_obj, client_id);

  client* prev = NULL;
  client* current = table_obj->buckets[bucket_idx];
  client* next = NULL;

  while (current != NULL) {
    next = current->next_hash;

    if (memcmp(current->id, client_id, HASH_LENGTH) == 0) {
      if (prev != NULL) {
        prev->next_hash = next;
      }
      else {
        table_obj->buckets[bucket_idx] = next;
      }

      return;
    }

    prev = current;
    current = next;
  }
}

size_t
get_bucket_idx(const client_hash_table* table_obj, const char* key)
{
  uint32_t hash = hash_xxhash(key);

  return (size_t) (hash % table_obj->size);
}

void
free_client_hash_table(client_hash_table* table_obj)
{
  /* Set all bucket pointers to null, remove list links */
  for (size_t i = 0; i < table_obj->size; ++i) {
    client* current = table_obj->buckets[i];
    client* next = NULL;

    while (current != NULL) {
      next = current->next_hash;
      if (next == NULL) break;

      current->next_hash = NULL;
      current = next;
    }

    table_obj->buckets[i] = NULL;
  }

  free(table_obj->buckets);
  free(table_obj);
}

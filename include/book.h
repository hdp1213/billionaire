#ifndef _BOOK_H_
#define _BOOK_H_

#include <stdlib.h>
#include <stdbool.h>

#include <json-c/json.h>

#include "card_location.h"
#include "utils.h"

#define OFFER_INDEX_OFFSET 2

typedef struct book book;
typedef struct offer offer;

/**
 * Struct storing current offers.
 */
struct book {
  /**
   * Zero-indexed array containing offers.
   */
  offer* offers[(TOTAL_COMMODITY_AMOUNT + 1) - OFFER_INDEX_OFFSET];
};

/**
 * Struct storing offers.
 */
struct offer {
  /**
   * ID of the owner of this offer.
   */
  char owner_id[HASH_LENGTH];

  /**
   * Cards involved in offer.
   */
  card_location* cards;
};

/**
 * Create a new empty book.
 */
book* book_new();

/**
 * Check if there is an offer already in the book at some index.
 */
bool offer_at(book* book_obj, int offer_ind);

/**
 * Check if there is no offer already in the book at some index.
 */
bool no_offer_at(book* book_obj, int offer_ind);

/**
 * Set an offer in the book at a given index to a given offer.
 */
void set_offer_at(book* book_obj, int offer_ind, offer* offer_obj);

/**
 * Get an offer from a book at a given index.
 */
offer* get_offer_at(book* book_obj, int offer_ind);

/**
 * Add an offer to the book, or return one that is ready to complete.
 */
offer* fill_offer(book* book_obj, offer* offer_obj);

/**
 * Free a book.
 */
void free_book(book* book_obj);


/**
 * Convert a offer struct to a JSON object offer.
 */
json_object* JSON_from_offer(offer* offer_obj);

/**
 * Convert a JSON object offer to a offer struct.
 */
offer* offer_from_JSON(json_object* offer_json);

/**
 * Create a new empty offer.
 */
offer* offer_new();

/**
 * Get the index of an offer corresponding to its place in a book.
 */
int get_offer_ind(offer* offer_obj);

/**
 * Free an offer.
 */
void free_offer(offer* offer_obj);

#endif

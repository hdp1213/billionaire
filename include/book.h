#ifndef _BOOK_H_
#define _BOOK_H_

#include <stdlib.h>
#include <stdbool.h>

#include <json-c/json.h>

#include "card_location.h"
#include "utils.h"

#define OFFER_INDEX_OFFSET 2
#define MAX_PARTICIPANTS 2

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
 * Remove an offer from the book.
 *
 * NOTE: this method does not free the removed offer from memory.
 */
void remove_offer_at(book* book_obj, int offer_ind);

/**
 * Add an offer to the book, or return one that is ready to complete.
 *
 * Sets cmd_errno to EOFFEROVER, on failure returns NULL.
 */
offer* fill_offer(book* book_obj, offer* offer_obj);

/**
 * Remove an offer from the book if it exists and belongs to the client.
 *
 * Sets cmd_errno to ECANEMPTY or ECANPERM, on failure returns NULL.
 */
offer* cancel_offer(book* book_obj, size_t card_amt, const char* client_id);

/**
 * Removes all current offers in book and frees associated memory.
 */
void clear_book(book* book_obj);

/**
 * Free a book.
 */
void free_book(book* book_obj);


/**
 * Convert an offer struct to a JSON object offer.
 *
 * Only used in check_book.c to confirm offer JSON works.
 */
json_object* JSON_from_offer(offer* offer_obj);

/**
 * Create a new empty offer.
 */
offer* offer_new();

/**
 * Initialise an offer struct with cards and the offer owner's ID.
 */
offer* offer_init(card_location* cards, const char* owner_id);

/**
 * Initialise an offer struct with a number of cards.
 *
 * Used for testing purposes only.
 */
offer* offer_init_cards(card_id card, size_t amount, const char* owner_id);

/**
 * Get the index of an offer corresponding to its place in a book.
 */
int get_offer_index(offer* offer_obj);

/**
 * Return the offset index corresponding to the number of cards in a trade.
 */
int offset_index(size_t card_amt);

/**
 * Check if a prospective owner actually owns the offer in question.
 */
bool is_owner(offer* offer_obj, const char* prospective_owner_id);

/**
 * Check if two offers have the same owner.
 */
bool have_same_owner(offer* offer1, offer* offer2);

/**
 * Free an offer.
 */
void free_offer(offer* offer_obj);

#endif

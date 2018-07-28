#include "book.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

#include "command_error.h"

book*
book_new()
{
  book* new_book = malloc(sizeof(book));

  if (new_book == NULL) {
    err(1, "new_book malloc failed");
  }

  for (int i = 0; i < (TOTAL_COMMODITY_AMOUNT + 1) - OFFER_INDEX_OFFSET; ++i) {
    new_book->offers[i] = NULL;
  }

  return new_book;
}

bool
offer_at(book* book_obj, int offer_ind)
{
  return book_obj->offers[offer_ind] != NULL;
}

bool
no_offer_at(book* book_obj, int offer_ind)
{
  return !offer_at(book_obj, offer_ind);
}

void
set_offer_at(book* book_obj, int offer_ind, offer* offer_obj)
{
  book_obj->offers[offer_ind] = offer_obj;
}

offer*
get_offer_at(book* book_obj, int offer_ind)
{
  return book_obj->offers[offer_ind];
}

void
remove_offer_at(book* book_obj, int offer_ind)
{
  set_offer_at(book_obj, offer_ind, NULL);
}

offer*
fill_offer(book* book_obj, offer* offer_obj)
{
  int offer_ind = get_offer_index(offer_obj);

  if (no_offer_at(book_obj, offer_ind)) {
    set_offer_at(book_obj, offer_ind, offer_obj);
    return NULL;
  }

  else {
    /* Return the offer object ready to trade to the parent method */
    offer* return_offer = get_offer_at(book_obj, offer_ind);

    if (have_same_owner(return_offer, offer_obj)) {
      cmd_errno = (int) EOFFEROVER;
      return NULL;
    }

    remove_offer_at(book_obj, offer_ind);
    return return_offer;
  }
}

offer*
cancel_offer(book* book_obj, size_t card_amt, const char* client_id)
{
  int offer_ind = offset_index(card_amt);

  if (no_offer_at(book_obj, offer_ind)) {
    /* No offer to cancel */
    cmd_errno = (int) ECANEMPTY;
    return NULL;
  }

  else {
    offer* offer_to_cancel = get_offer_at(book_obj, offer_ind);

    if (is_owner(offer_to_cancel, client_id)) {
      remove_offer_at(book_obj, offer_ind);
      return offer_to_cancel;
    }

    else {
      /* Client does not own offer to cancel */
      cmd_errno = (int) ECANPERM;
      return NULL;
    }
  }
}

void
clear_book(book* book_obj)
{
  for (int i = 0; i < (TOTAL_COMMODITY_AMOUNT + 1) - OFFER_INDEX_OFFSET; ++i) {
    if (offer_at(book_obj, i)) {
      free_offer(book_obj->offers[i]);
      book_obj->offers[i] = NULL;
    }
  }
}

void
free_book(book* book_obj)
{
  for (int i = 0; i < (TOTAL_COMMODITY_AMOUNT + 1) - OFFER_INDEX_OFFSET; ++i) {
    if (offer_at(book_obj, i)) {
      free_offer(book_obj->offers[i]);
    }
  }

  free(book_obj);
}


json_object*
JSON_from_offer(offer* offer_obj)
{
  /* The client does not verify it is the owner of the offer, so the JSON
     object does not need to contain the owner_id field. */
  json_object* offer_json = json_object_new_object();

  json_object* offer_cards_json = JSON_from_card_location(offer_obj->cards);
  json_object_object_add(offer_json, "cards", offer_cards_json);

  return offer_json;
}

offer*
offer_new()
{
  offer* new_offer = malloc(sizeof(offer));

  if (new_offer == NULL) {
    err(1, "new_offer malloc failed");
  }

  strncpy(new_offer->owner_id, "", 2);
  new_offer->cards = NULL;

  return new_offer;
}

offer*
offer_init(card_location* cards, const char* owner_id)
{
  offer* offer_obj = offer_new();

  /* Assume owner_id has length HASH_LENGTH, as it should come from utils.h */
  strncpy(offer_obj->owner_id, owner_id, HASH_LENGTH);

  offer_obj->cards = cards;

  return offer_obj;
}

offer*
offer_init_cards(card_id card, size_t amount, const char* owner_id)
{
  card_location* cards = card_location_new();
  add_cards_to_location(cards, card, amount);

  return offer_init(cards, owner_id);
}


int
get_offer_index(offer* offer_obj)
{
  size_t card_amt = get_total_cards(offer_obj->cards);

  return offset_index(card_amt);
}

int
offset_index(size_t card_amt)
{
  /* Already asserted that num_cards >= OFFER_INDEX_OFFSET */
  return (int) card_amt - OFFER_INDEX_OFFSET;
}

bool
is_owner(offer* offer_obj, const char* prospective_owner_id)
{
  return strncmp(offer_obj->owner_id, prospective_owner_id, HASH_LENGTH) == 0;
}

bool
have_same_owner(offer* offer1, offer* offer2)
{
  return is_owner(offer1, offer2->owner_id);
}

void
free_offer(offer* offer_obj)
{
  free_card_location(offer_obj->cards);
  free(offer_obj);
}

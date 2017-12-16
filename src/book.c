#include <err.h>
#include <stdio.h>

#include "book.h"

book* book_new()
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
free_book(book* book_obj)
{
  for (int i = 0; i < (TOTAL_COMMODITY_AMOUNT + 1) - OFFER_INDEX_OFFSET; ++i) {
    if (offer_at(book_obj, i)) {
      free_offer(book_obj->offers[i]);
    }
  }

  free(book_obj);
}

offer*
fill_offer(book* book_obj, offer* offer_obj)
{
  int offer_ind = get_offer_ind(offer_obj);

  if (no_offer_at(book_obj, offer_ind)) {
    set_offer_at(book_obj, offer_ind, offer_obj);
    return NULL;
  }
  else {
    /* Return the offer object ready to trade to the parent method */
    return get_offer_at(book_obj, offer_ind);
  }
}


json_object*
JSON_from_offer(offer* offer_obj)
{
  json_object* offer_json = json_object_new_object();

  json_object* offer_cards_json = JSON_from_card_location(offer_obj->cards);
  json_object* id_json = json_object_new_string(offer_obj->owner_id);

  json_object_object_add(offer_json, "cards", offer_cards_json);
  json_object_object_add(offer_json, "client_id", id_json);

  return offer_json;
}

offer*
offer_from_JSON(json_object* offer_json)
{
  offer* offer_obj = offer_new();

  json_object* offer_cards_json = get_JSON_value(offer_json, "cards");
  json_object* id_json = get_JSON_value(offer_json, "client_id");

  offer_obj->cards = card_location_from_JSON(offer_cards_json);

  /* Assume client_id has length HASH_LENGTH, as it should come from utils.h */
  const char* client_id = json_object_get_string(id_json);
  strncpy(offer_obj->owner_id, client_id, HASH_LENGTH);

  return offer_obj;
}

offer*
offer_new()
{
  offer* new_offer = malloc(sizeof(offer));

  if (new_offer == NULL) {
    err(1, "new_offer malloc failed");
  }

  new_offer->cards = NULL;

  return new_offer;
}

int
get_offer_ind(offer* offer_obj)
{
  size_t card_amt = get_total_cards(offer_obj->cards);

  /* Already asserted that num_cards >= OFFER_INDEX_OFFSET */
  return (int) card_amt - OFFER_INDEX_OFFSET;
}

void
free_offer(offer* offer_obj)
{
  free_card_location(offer_obj->cards);
  free(offer_obj);
}

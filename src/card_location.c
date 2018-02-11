#include <err.h>
#include <stdarg.h>
#include <stdio.h>

#include "card_location.h"
#include "command_error.h"
#include "utils.h"


json_object*
JSON_from_card_location(card_location* card_loc)
{
  json_object* card_loc_json = json_object_new_array();

  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    size_t card_amt = get_card_amount(card_loc, card);

    if (card_amt == 0) {
      continue;
    }

    json_object* card_json = json_object_new_object();

    json_object* card_id_json = json_object_new_int(card);
    json_object* card_amt_json = json_object_new_int((int) card_amt);

    json_object_object_add(card_json, "id", card_id_json);
    json_object_object_add(card_json, "amt", card_amt_json);

    json_object_array_add(card_loc_json, card_json);
  }

  return card_loc_json;
}

card_location*
card_location_from_JSON(json_object* card_loc_json)
{
  if (!json_object_is_type(card_loc_json, json_type_array)) {
    cmd_errno = (int) EJSONTYPE;
    return NULL;
  }

  card_location* card_loc = card_location_new();

  /* Iterate through each array item */
  size_t array_len = json_object_array_length(card_loc_json);

  for (size_t i = 0; i < array_len; ++i) {
    json_object* card_json = json_object_array_get_idx(card_loc_json, i);

    json_object* card_id_json = get_JSON_value(card_json, "id");

    if (cmd_errno != CMD_SUCCESS) {
      return NULL;
    }

    json_object* card_amt_json = get_JSON_value(card_json, "amt");

    if (cmd_errno != CMD_SUCCESS) {
      return NULL;
    }

    card_id card = json_object_get_int(card_id_json);
    size_t card_amt = (size_t) json_object_get_int(card_amt_json);

    add_cards_to_location(card_loc, card, card_amt);
  }

  return card_loc;
}

card_location*
card_location_new()
{
  card_location* new_card_location = malloc(sizeof(card_location));

  if (new_card_location == NULL) {
    err(1, "new_card_location malloc failed");
  }

  /* Create array used to count the number of cards, initialised to zero */
  new_card_location->card_counts = calloc(TOTAL_UNIQUE_CARDS, sizeof(size_t));

  if (new_card_location->card_counts == NULL) {
    err(1, "new_card_location->card_counts malloc failed");
  }

  /* Initialise other fields */
  new_card_location->num_cards = 0;

  return new_card_location;
}

card_location*
card_location_init(size_t num_cards, ...)
{
  card_location* new_card_location = card_location_new();

  /* Initialise card_counts with the list of new cards */
  va_list card_list;
  va_start(card_list, num_cards);

  for (size_t i = 0; i < num_cards; ++i) {
    card_id card = va_arg(card_list, card_id);

    add_card_to_location(new_card_location, card);
  }

  va_end(card_list);

  return new_card_location;
}

card_location*
generate_deck(int num_players, bool has_billionaire, bool has_tax_collector)
{
  card_location* new_deck = card_location_new();

  /* Get the last commodity card for the amount of players playing */
  card_id last_comm = (card_id) num_players;

  /* The number of commodity cards added to a new deck is always equal to the
     number of unique commodities plus one */
  for (card_id comm_card = DIAMONDS; comm_card < last_comm; ++comm_card) {
    size_t amount = TOTAL_COMMODITY_AMOUNT + 1;
    add_cards_to_location(new_deck, comm_card, amount);
  }

  if (has_billionaire) {
    add_card_to_location(new_deck, BILLIONAIRE);
  }

  if (has_tax_collector) {
    add_card_to_location(new_deck, TAX_COLLECTOR);
  }

  return new_deck;
}

void
add_card_to_location(card_location* card_loc, card_id card)
{
  add_cards_to_location(card_loc, card, 1);
}

void
remove_card_from_location(card_location* card_loc, card_id card)
{
  remove_cards_from_location(card_loc, card, 1);
}

void
add_cards_to_location(card_location* card_loc, card_id card, size_t amount)
{
  card_loc->num_cards += amount;
  card_loc->card_counts[card] += amount;
}

void
remove_cards_from_location(card_location* card_loc, card_id card, size_t amount)
{
  if (has_enough_cards(card_loc, card, amount)) {
    card_loc->num_cards -= amount;
    card_loc->card_counts[card] -= amount;
  }
  else {
    /* Handle not having enough cards to remove */
  }
}

void
merge_card_location(card_location* dest_loc, const card_location* src_loc)
{
  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    size_t card_amt = get_card_amount(src_loc, card);

    add_cards_to_location(dest_loc, card, card_amt);
  }
}

void
subtract_card_location(card_location* dest_loc, const card_location* src_loc)
{
  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    size_t card_amt = get_card_amount(src_loc, card);

    if (has_enough_cards(dest_loc, card, card_amt)) {
      remove_cards_from_location(dest_loc, card, card_amt);
    }
    else {
      /* TODO: handle error */
    }
  }
}

void
check_offer_subset(const card_location* offer, const card_location* hand)
{
  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    size_t offer_amt = get_card_amount(offer, card);

    if (!has_enough_cards(hand, card, offer_amt)) {
      cmd_errno = (int) EBADCARDS;
      return;
    }
  }
}

size_t
get_card_amount(const card_location* card_loc, card_id card)
{
  return card_loc->card_counts[card];
}

size_t
get_total_cards(const card_location* card_loc)
{
  return card_loc->num_cards;
}

bool
has_enough_cards(const card_location* card_loc, card_id card, size_t amount)
{
  size_t amount_at_location = get_card_amount(card_loc, card);

  return (amount <= amount_at_location);
}

void
clear_card_location(card_location* card_loc)
{
  card_loc->num_cards = 0;

  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    card_loc->card_counts[card] = 0;
  }
}

void
move_cards(card_location* from_loc, card_location* to_loc, card_id card, size_t amount)
{
  /* Only add cards to to_loc if they can be removed from from_loc */
  if (has_enough_cards(from_loc, card, amount)) {
    remove_cards_from_location(from_loc, card, amount);
    add_cards_to_location(to_loc, card, amount);
  }
  else {
    /* Handle not having enough cards to remove */
  }
}

void
free_card_location(card_location* card_loc)
{
  free(card_loc->card_counts);
  free(card_loc);
}

#include "card_location.h"

#include <err.h>
#include <stdarg.h>
#include <stdio.h>

#include "command_error.h"
#include "utils.h"

const int card_values[] = {
  700, /* DIAMONDS */
  500, /* GOLD */
  800, /* OIL */
  200, /* PROPERTY */
  400, /* MINING */
  300, /* SHIPPING */
  600, /* BANKING */
  100, /* SPORT */
  0,   /* BILLIONAIRE */
  -200 /* TAX_COLLECTOR */
};

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
    json_object* card_value_json = json_object_new_int(card_values[card]);

    json_object_object_add(card_json, "id", card_id_json);
    json_object_object_add(card_json, "amt", card_amt_json);
    json_object_object_add(card_json, "val", card_value_json);

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
      free_card_location(card_loc);
      return NULL;
    }

    json_object* card_amt_json = get_JSON_value(card_json, "amt");

    if (cmd_errno != CMD_SUCCESS) {
      free_card_location(card_loc);
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
    cmd_errno = (int) ECARDRM;
    return;
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

    remove_cards_from_location(dest_loc, card, card_amt);

    if (cmd_errno != CMD_SUCCESS) {
      return;
    }
  }
}

void
validate_offer(card_location* offer, const card_location* hand)
{
  size_t total_offer_size = get_total_cards(offer);

  /* Check if any cards were traded */
  if (total_offer_size == 0) {
    cmd_errno = (int) ENOOFFER;
    free_card_location(offer);
    return;
  }

  /* Check if enough cards were traded */
  else if (total_offer_size < OFFER_MIN_CARDS) {
    cmd_errno = (int) ESMALLOFFER;
    /* Offer not freed as it needs to be sent back */
    return;
  }

  /* Individual card type checking */
  size_t num_commodities = 0, num_wildcards = 0;

  for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
    size_t card_amt = get_card_amount(offer, card);

    if (card_amt > 0) {
      /* Check offer is a subset of the hand */
      if (!has_enough_cards(hand, card, card_amt)) {
        cmd_errno = (int) EHANDSUBSET;
        /* Offer not freed as it needs to be sent back */
        return;
      }

      /* Card type is commodity */
      if (card < TOTAL_COMMODITY_AMOUNT) {
        num_commodities++;

        /* Check offer contains <= OFFER_MAX_UNIQ_COMMS unique commodities */
        if (num_commodities > OFFER_MAX_UNIQ_COMMS) {
          cmd_errno = (int) EUNIQCOMMS;
          /* Offer not freed as it needs to be sent back */
          return;
        }
      }
      /* Card type is wildcard */
      else {
        num_wildcards++;

        /* Check offer contains <= OFFER_MAX_UNIQ_WILDS unique wildcards */
        if (num_wildcards > OFFER_MAX_UNIQ_WILDS) {
          cmd_errno = (int) EUNIQWILDS;
          /* Offer not freed as it needs to be sent back */
          return;
        }
      }
    }
  }
}

bool
has_won(const card_location* hand)
{
  /* Check for wildcards */
  size_t num_wildcards = 0;
  num_wildcards += get_card_amount(hand, BILLIONAIRE);
  num_wildcards += get_card_amount(hand, TAX_COLLECTOR);

  /* Check win condition */
  for (card_id comm_card = DIAMONDS; comm_card < TOTAL_COMMODITY_AMOUNT; ++comm_card) {
    size_t comm_amt = get_card_amount(hand, comm_card);

    if ((comm_amt + num_wildcards) >= (TOTAL_COMMODITY_AMOUNT + 1)) {
      return true;
    }
  }

  return false;
}

int
evaluate_hand_score(const card_location* hand)
{
  int score = 0;
  size_t num_wildcards = 0;
  num_wildcards += get_card_amount(hand, BILLIONAIRE);
  num_wildcards += get_card_amount(hand, TAX_COLLECTOR);

  /* Subtract points if hand contains TAX_COLLECTOR */
  if (get_card_amount(hand, TAX_COLLECTOR) > 0) {
    score += card_values[TAX_COLLECTOR];
  }

  /* Add total for winning commodity type */
  for (card_id comm_card = DIAMONDS; comm_card < TOTAL_COMMODITY_AMOUNT; ++comm_card) {
    size_t comm_amt = get_card_amount(hand, comm_card);

    if ((comm_amt + num_wildcards) >= (TOTAL_COMMODITY_AMOUNT + 1)) {
      score += card_values[comm_card];
    }
  }

  /* Double total if hand contains BILLIONAIRE */
  if (get_card_amount(hand, BILLIONAIRE) > 0) {
    score *= 2;
  }

  return score;
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
free_card_location(card_location* card_loc)
{
  free(card_loc->card_counts);
  free(card_loc);
}

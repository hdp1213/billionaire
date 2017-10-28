#include <stdio.h>
#include <errno.h>
#include <err.h>

#include "card.h"

const commodity_type ALL_COMMODITIES[] = {
  DIAMONDS, GOLD, OIL, PROPERTY, MINING, SHIPPING, BANKING, SPORT
};

json_object*
card_to_JSON(card* card_obj)
{
  json_object* card_json = json_object_new_object();

  json_object* commodity = json_object_new_int((int) card_obj->commodity);
  json_object* type = json_object_new_int((int) card_obj->type);

  json_object_object_add(card_json, "type", type);
  json_object_object_add(card_json, "commodity", commodity);

  return card_json;
}

card*
card_new(card_type type, commodity_type commodity)
{
  card* new_card = malloc(sizeof(card*));

  if (new_card == NULL) {
    err(1, "malloc failed");
  }

  new_card->type = type;
  new_card->commodity = commodity;

  return new_card;
}

card**
generate_deck(int num_players, bool has_billionaire,
              bool has_taxman, size_t* num_cards)
{
  size_t i = 0;

  /* Get total number of cards to play with */
  *num_cards = num_players * COMMODITY_AMOUNT;
  *num_cards += has_billionaire ? 1 : 0;
  *num_cards += has_taxman ? 1 : 0;

  /* Allocate array of new cards */
  card** new_cards = calloc(*num_cards, sizeof(card**));

  if (new_cards == NULL) {
    err(1, "malloc failed");
  }

  /* Generate commodity cards */
  for (int icom = 0; icom < num_players; ++icom) {
    commodity_type current_commodity = ALL_COMMODITIES[icom];

    for (int icard = 0; icard < COMMODITY_AMOUNT; ++icard) {
      /* Each new card should have a different memory address */
      card* new_commodity_card = card_new(COMMODITY, current_commodity);
      new_cards[i] = new_commodity_card;
      i++;
    }
  }

  /* Generate special cards, if any are needed */
  if (has_billionaire) {
    card* new_billionaire_card = card_new(BILLIONAIRE, NONE);
    new_cards[i] = new_billionaire_card;
    i++;
  }

  if (has_taxman) {
    card* new_taxman_card = card_new(TAXMAN, NONE);
    new_cards[i] = new_taxman_card;
    i++;
  }

#ifdef DBUG
  printf("Generated %d/%d new cards\n", i, *num_cards);
#endif

  return new_cards;
}

void
free_cards(card** cards, size_t num_cards)
{
  /* Free each card */
  for (size_t i = 0; i < num_cards; ++i) {
    free(cards[i]);
  }

  /* Free the containing card array */
  free(cards);
}

void
shuffle_cards(card** cards, size_t num_cards)
{
  if (num_cards > 1) {
    for (size_t i = 0; i < num_cards - 1; ++i) {
      size_t rnd = (size_t) rand();
      size_t j = i + rnd/(RAND_MAX/(num_cards - i) + 1);

      card* tmp = cards[j];
      cards[j] = cards[i];
      cards[i] = tmp;
    }
  }
}

void
display_cards(card** cards, size_t num_cards)
{
  for (size_t i = 0; i < num_cards; ++i) {
    printf("Card %d:\n", i);
    printf("\tType: %d\n", (int) cards[i]->type);
    printf("\tCommodity: %d\n", (int) cards[i]->commodity);
  }
}

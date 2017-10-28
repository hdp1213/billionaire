#include <stdio.h>
#include <errno.h>
#include <err.h>

#include "card.h"

const commodity_type ALL_COMMODITIES[] = {
  DIAMONDS, GOLD, OIL, PROPERTY, MINING, SHIPPING, BANKING, SPORT
};

json_object*
card_to_json(card* card_obj)
{
  json_object* card_json = json_object_new_object();

  json_object* commodity = json_object_new_int((int) card_obj->commodity);
  json_object* type = json_object_new_int((int) card_obj->type);

  json_object_object_add(card_json, "commodity", commodity);
  json_object_object_add(card_json, "type", type);

  return card_json;
}

card**
generate_cards(int num_players, bool has_billionaire,
               bool has_taxman, size_t* num_cards) {
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
      card* new_commodity_card = malloc(sizeof(card*));

      if (new_commodity_card == NULL) {
        err(1, "malloc failed");
      }

      new_commodity_card->type = COMMODITY;
      new_commodity_card->commodity = current_commodity;

      new_cards[i] = new_commodity_card;
      i++;
    }
  }

  /* Generate special cards, if any are needed */
  if (has_billionaire) {
    card* new_billionaire_card = malloc(sizeof(card*));

    if (new_billionaire_card == NULL) {
      err(1, "malloc failed");
    }

    new_billionaire_card->type = BILLIONAIRE;
    new_billionaire_card->commodity = NONE;

    new_cards[i] = new_billionaire_card;
    i++;
  }

  if (has_taxman) {
    card* new_taxman_card = malloc(sizeof(card*));

    if (new_taxman_card == NULL) {
      err(1, "malloc failed");
    }

    new_taxman_card->type = TAXMAN;
    new_taxman_card->commodity = NONE;

    new_cards[i] = new_taxman_card;
    i++;
  }

#ifdef DBUG
  printf("Generated %d/%d new cards\n", i, *num_cards);
#endif

  return new_cards;
}

void
free_cards(card** cards, size_t num_cards) {
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

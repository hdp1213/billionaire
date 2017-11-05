#include <errno.h>
#include <err.h>
#include <stdio.h>

#include "card.h"
#include "utils.h"

const commodity_type ALL_COMMODITIES[] = {
  DIAMONDS, GOLD, OIL, PROPERTY, MINING, SHIPPING, BANKING, SPORT
};

json_object*
JSON_from_card(card* card_obj)
{
  json_object* card_json = json_object_new_object();

  json_object* type = json_object_new_int((int) card_obj->type);
  json_object* commodity = json_object_new_int((int) card_obj->commodity);

  json_object_object_add(card_json, "type", type);
  json_object_object_add(card_json, "commodity", commodity);

  return card_json;
}

card*
card_from_JSON(json_object* json_card_obj)
{
  card_type type;
  commodity_type commodity;

  json_object* json_type = get_JSON_value(json_card_obj, "type");
  json_object* json_commodity = get_JSON_value(json_card_obj, "commodity");

  type = (card_type) json_object_get_int(json_type);
  commodity = (commodity_type) json_object_get_int(json_commodity);

  return card_new(type, commodity);
}

card*
card_new(card_type type, commodity_type commodity)
{
  card* new_card = malloc(sizeof(card));

  if (new_card == NULL) {
    err(1, "new_card malloc failed");
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
  card** new_cards = calloc(*num_cards, sizeof(card*));

  if (new_cards == NULL) {
    err(1, "new_cards malloc failed");
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
  }

#ifdef DBUG
  printf("Generated %zu/%zu new cards\n", i, *num_cards);
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
    printf("Card %zu:\n", i);
    printf("\tType: %d\n", (int) cards[i]->type);
    printf("\tCommodity: %d\n", (int) cards[i]->commodity);
  }
}

void
deal_cards(size_t num_players, card** deck, size_t deck_size,
           card**** player_hands, size_t** player_hand_sizes)
{
  /* Allocate a "hand" for each player */
  *player_hands = calloc(num_players, sizeof(card**));

  if (*player_hands == NULL) {
    err(1, "*player_hands malloc failed");
  }

  /* Also allocate the array going to hold the size of each player's
     hand. Do a zero initialisation */
  *player_hand_sizes = calloc(num_players, sizeof(size_t));

  if (*player_hand_sizes == NULL) {
    err(1, "*player_hand_sizes malloc failed");
  }

  /* Each hand has a maximum size of MAX_HAND_SIZE */
  for (size_t i = 0; i < num_players; ++i) {
    (*player_hands)[i] = calloc(MAX_HAND_SIZE, sizeof(card*));

    if ((*player_hands)[i] == NULL) {
      err(1, "(*player_hands)[%d] malloc failed", i);
    }
  }

  /* Now, deal the cards out */
  for (size_t i = 0; i < deck_size; ++i) {
    /* Get the current player */
    size_t iplayer = i % num_players;
#ifdef DBUG
    printf("Player %zu, card %zu\n", iplayer, i);
#endif

    /* Give a card to a player, using player_hand_sizes to track the
       position of each new card in the hand. */
    (*player_hands)[iplayer][(*player_hand_sizes)[iplayer]] = deck[i];

    /* Increment the player's hand's size by one */
    (*player_hand_sizes)[iplayer]++;
  }
}

void
free_player_hands(card*** player_hands, size_t num_players)
{
  /* Freeing player_hands[iplayer][i] for
   * i < player_hand_sizes[iplayer] would free memory associated with
   * cards. This action is reserved for a free_cards() call on the
   * original deck array instead.
   */
  for (size_t iplayer = 0; iplayer < num_players; ++iplayer) {
    free(player_hands[iplayer]);
  }

  free(player_hands);
}

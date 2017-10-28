#include "billionaire.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>

const struct commands Command = {
  "JOIN", "START", "RECEIVE", "CHECK", "FINISH", "ASK", "CANCEL", "BILLIONAIRE"
};

const commodity_type ALL_COMMODITIES[] = {
  DIAMONDS, GOLD, OIL, PROPERTY, MINING, SHIPPING, BANKING, SPORT
};

json_object*
make_command(const char* cmd_name)
{
  json_object* cmd = json_object_new_object();
  json_object* cmd_str = json_object_new_string(cmd_name);
  json_object_object_add(cmd, "command", cmd_str);

  return cmd;
}

json_object*
billionaire_join(const char* addr, size_t length, char** id)
{
  json_object* cmd = make_command(Command.JOIN);

  *id = hash_addr(addr, length);

  json_object* bot_id = json_object_new_string(*id);
  json_object_object_add(cmd, "bot_id", bot_id);

  return cmd;
}

json_object*
billionaire_start(card** player_cards, size_t num_cards)
{
  json_object* cmd = make_command(Command.START);

  json_object* hand = json_object_new_array();

  for (size_t i = 0; i < num_cards; ++i) {
    json_object* card = card_to_json(player_cards[i]);
    json_object_array_add(hand, card);
  }

  json_object_object_add(cmd, "hand", hand);

  return cmd;
}

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

game_state*
game_state_new(int player_limit, bool has_billionaire, bool has_taxman)
{
  game_state* new_game_state = malloc(sizeof(game_state*));

  if (new_game_state == NULL) {
    err(1, "malloc failed");
  }

  /* Initialise parameter values */
  new_game_state->num_players = 0;
  new_game_state->player_limit = player_limit;

  /* Initialise deck */
  new_game_state->deck = generate_cards(player_limit,
                                        has_billionaire,
                                        has_taxman,
                                        &new_game_state->deck_size);

  /* Shuffle the deck */
  shuffle_cards(new_game_state->deck,
                new_game_state->deck_size);

  return new_game_state;
}

void
game_state_free(game_state* gs_obj)
{
  free_cards(gs_obj->deck, gs_obj->deck_size);
  free(gs_obj);
}

const char*
stringify_command(json_object* json_obj, size_t* str_len)
{
  return json_object_to_json_string_length(json_obj,
                                           JSON_C_TO_STRING_PLAIN,
                                           str_len);
}

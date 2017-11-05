#include <mcheck.h>
#include <stdio.h>

#include "card.h"
#include "utils.h"

void
mem_check_card_JSON()
{
  card* new_card;
  json_object* card_json;

  new_card = card_new(COMMODITY, DIAMONDS);
  card_json = JSON_from_card(new_card);

  free(new_card);
  json_object_put(card_json);
}

void
mem_check_deck_JSON()
{
  size_t num_players = 8;
  bool has_billionaire = true;
  bool has_taxman = true;
  size_t deck_size = 0;

  const char* total_str;
  size_t total_str_len;

  card** deck = generate_deck(num_players, has_billionaire, has_taxman,
                              &deck_size);

  json_object* total = json_object_new_object();
  json_object* hand = json_object_new_array();

  for (size_t i = 0; i < deck_size; ++i) {
    json_object* card = JSON_from_card(deck[i]);
    json_object_array_add(hand, card);
  }

  json_object_object_add(total, "hand", hand);

  total_str = JSON_to_str(total, &total_str_len);

  printf("%s\n", total_str);

  free_cards(deck, deck_size);
  json_object_put(total);
}

void
mem_check_dealing()
{
  size_t num_players = 8;
  bool has_billionaire = true;
  bool has_taxman = true;
  size_t deck_size = 0;

  card** deck = generate_deck(num_players, has_billionaire, has_taxman,
                              &deck_size);

  card*** player_hands;
  size_t* player_hand_sizes;

  deal_cards(num_players, deck, deck_size,
             &player_hands, &player_hand_sizes);

  free(player_hand_sizes);
  free_player_hands(player_hands, num_players);
  free_cards(deck, deck_size);
}

int
main()
{
  mtrace();

  /* Check card memory allocation */
  mem_check_card_JSON();
  mem_check_deck_JSON();

  mem_check_dealing();
}

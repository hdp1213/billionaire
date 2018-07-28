#include "billionaire.h"

#include <stdio.h>

#include "book.h"
#include "card_location.h"
#include "client.h"
#include "client_hash_table.h"
#include "command.h"
#include "command_error.h"
#include "game_state.h"
#include "utils.h"

void
start_billionaire_game()
{
  client* client_obj = NULL;

  printf("Player limit of %d reached. Game starting...\n",
         billionaire_game->player_limit);
  billionaire_game->running = true;

  card_location** player_hands;

  printf("Dealing cards...\n");
  player_hands = deal_cards(billionaire_game->num_players,
                            billionaire_game->deck);

  size_t iplayer = 0;

  /* Split the deck between all players, and send their hands through START */
  TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
    json_object* start = command_start(player_hands[iplayer]);
    client_obj->hand = player_hands[iplayer];

    enqueue_command(client_obj, start);
    iplayer++;
  }

  free(player_hands);
}

void
stop_billionaire_game()
{
  client* client_obj = NULL;

  printf("Player limit of %d no longer satisfied. Game stopping...\n",
         billionaire_game->player_limit);
  billionaire_game->running = false;

  /* Clear current trade book of current trades */
  clear_book(billionaire_game->current_trades);

  /* Send an END_GAME command to each remaining client */
  TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
    json_object* end_game = command_end_game();
    enqueue_command(client_obj, end_game);
  }

}

void
process_client_command(client* this_client, char json_str[], size_t str_size)
{
  client* client_obj = NULL;
  json_object* cmd_array = parse_command_list_string(json_str, str_size);

  if (cmd_errno != CMD_SUCCESS) {
    enqueue_command(this_client, command_error());
  }

  else {
    JSON_ARRAY_FOREACH(cmd_obj, cmd_array) {
      /* Check cmd_object has command field */
      if (get_JSON_value(cmd_obj, "command") == NULL) {
        cmd_errno = (int) EBADCMDOBJ;
        enqueue_command(this_client, command_error());
        continue;
      }

      if (command_is(cmd_obj, Command.NEW_OFFER)) {
        printf("Received NEW_OFFER from %s\n", this_client->id);

        /* Parse offer */
        json_object* card_array = get_JSON_value(cmd_obj, "cards");

        if (cmd_errno != CMD_SUCCESS) {
          enqueue_command(this_client, command_error());
          continue;
        }

        card_location* card_loc = card_location_from_JSON(card_array);

        if (cmd_errno != CMD_SUCCESS) {
          enqueue_command(this_client, command_error());
          continue;
        }

        /* Validate the offer */
        validate_offer(card_loc, this_client->hand);

        if (cmd_errno != CMD_SUCCESS) {
          if (cmd_errno != ENOOFFER) {
            /* Send CANCELLED_OFFER back to this_client */
            offer* bad_offer = offer_init(card_loc, this_client->id);

            json_object* cancel = command_cancelled_offer(bad_offer);
            enqueue_command(this_client, cancel);

            free_offer(bad_offer);
          }

          enqueue_command(this_client, command_error());
          continue;
        }

        size_t total_cards = get_total_cards(card_loc);

        printf("Offer of %zu cards\n", total_cards);

#ifdef DBUG
        for (card_id card = DIAMONDS; card < TOTAL_UNIQUE_CARDS; ++card) {
          size_t card_amt = get_card_amount(card_loc, card);

          if (card_amt == 0) {
            continue;
          }

          printf("  %zux of card %d\n", card_amt, card);
        }
#endif /* DBUG */

        /* Add offer to book */
        offer* new_offer = offer_init(card_loc, this_client->id);
        offer* traded_offer = fill_offer(billionaire_game->current_trades,
                                         new_offer);

        if (cmd_errno != CMD_SUCCESS) {
          /* Send CANCELLED_OFFER back to this_client */
          json_object* cancel = command_cancelled_offer(new_offer);
          enqueue_command(this_client, cancel);

          free_offer(new_offer);

          enqueue_command(this_client, command_error());
          continue;
        }

        /* Update this_client's hand */
        subtract_card_location(this_client->hand, new_offer->cards);

        if (cmd_errno != CMD_SUCCESS) {
          enqueue_command(this_client, command_error());
          /* TODO: send offer back? */
          free_offer(new_offer);
          continue;
        }

        /* Check if an offer has traded */
        if (traded_offer != NULL) {
          client* other_client = get_client(hashed_clients, traded_offer->owner_id);

          /* Update participants' hands */
          merge_card_location(this_client->hand, traded_offer->cards);
          merge_card_location(other_client->hand, new_offer->cards);

          /* Send SUCCESSFUL_TRADE commands to participants */
          json_object* this_trade = command_successful_trade(traded_offer);
          json_object* other_trade = command_successful_trade(new_offer);

          free_offer(new_offer);
          free_offer(traded_offer);

          enqueue_command(this_client, this_trade);
          enqueue_command(other_client, other_trade);

          /* Send BOOK_EVENT to remaining players */
          const char* participants[MAX_PARTICIPANTS] = {this_client->id, other_client->id};

          /* Check for win conditions */
          bool this_client_has_won = has_won(this_client->hand);
          bool other_client_has_won = has_won(other_client->hand);

          TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
            if (this_client_has_won) {
              enqueue_command(client_obj, command_billionaire(this_client->id));
            }

            if (other_client_has_won) {
              enqueue_command(client_obj, command_billionaire(other_client->id));
            }

            if (client_eq(client_obj, this_client) ||
                client_eq(client_obj, other_client)) {
              continue;
            }

            json_object* book_event = command_book_event(Command.SUCCESSFUL_TRADE,
                                                         total_cards,
                                                         participants);

            enqueue_command(client_obj, book_event);
          }

          /* TODO: Reset the round */
          if (this_client_has_won || other_client_has_won) {
            /* Update each client's score */
            printf("Updating scores...\n");
            TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
              update_score(client_obj);
#ifdef DBUG
              printf("%s's score is now %d\n",
                     client_obj->id, client_obj->score);
#endif /* DBUG */
              enqueue_command(client_obj,
                              command_end_round(client_obj->score));
            }

            printf("Clearing book...\n");
            clear_book(billionaire_game->current_trades);
          }
        }

        else {
          printf("Offer added to book\n");

          /* Send BOOK_EVENT to remaining players */
          const char* participants[MAX_PARTICIPANTS] = {this_client->id, NULL};

          TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
            if (client_eq(client_obj, this_client)) {
              continue;
            }

            json_object* book_event = command_book_event(Command.NEW_OFFER,
                                                         total_cards,
                                                         participants);

            enqueue_command(client_obj, book_event);
          }
        }
      } /* Command.NEW_OFFER */

      else if (command_is(cmd_obj, Command.CANCEL_OFFER)) {
        printf("Received CANCEL_OFFER from %s\n", this_client->id);

        /* Parse offer */
        json_object* card_amt_json = get_JSON_value(cmd_obj, "card_amt");

        if (cmd_errno != CMD_SUCCESS) {
          enqueue_command(this_client, command_error());
          continue;
        }

        size_t card_amt = (size_t) json_object_get_int(card_amt_json);

        offer* cancelled_offer = cancel_offer(billionaire_game->current_trades,
                                              card_amt, this_client->id);

        if (cmd_errno != CMD_SUCCESS) {
          enqueue_command(this_client, command_error());
          continue;
        }

        /* Offer has been successfully cancelled */
        json_object* cancel = command_cancelled_offer(cancelled_offer);
        enqueue_command(this_client, cancel);

        merge_card_location(this_client->hand, cancelled_offer->cards);

        /* Send BOOK_EVENT to remaining players */
        const char* participants[MAX_PARTICIPANTS] = {this_client->id, NULL};

        TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
          if (client_eq(client_obj, this_client)) {
            continue;
          }

          json_object* book_event = command_book_event(Command.CANCELLED_OFFER,
                                                           card_amt,
                                                           participants);

          enqueue_command(client_obj, book_event);
        }
      } /* Command.CANCEL_OFFER */

      else {
        /* Invalid command name */
        cmd_errno = (int) EBADCMDNAME;
        enqueue_command(this_client, command_error());
        continue;
      }
    }
  }

  /* Free cmd_array after use */
  json_object_put(cmd_array);

  /* Send any outstanding commands to clients */
  send_commands_to_clients(&client_tailq_head);
}

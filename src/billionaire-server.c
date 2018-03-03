/*
 * The MIT License (MIT)
 * Copyright (c) 2011 Jason Ish
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * A simple chat server using libevent.
 *
 * @todo Check API usage with libevent2 proper API usage.
 * @todo IPv6 support - using libevent socket helpers, if any.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

#include <getopt.h>
#include <signal.h>

#include "billionaire-server.h"
#include "book.h"
#include "card_location.h"
#include "card_array.h"
#include "client.h"
#include "command_error.h"
#include "utils.h"

#define READ_BYTES_AMOUNT 8192

int
setnonblock(int fd)
{
  int flags;

  flags = fcntl(fd, F_GETFL);
  if (flags < 0)
    return flags;
  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0)
    return -1;

  return 0;
}

void
buffered_on_read(struct bufferevent* bev, void* arg)
{
  client* this_client = (client*) arg;
  client* client_obj = NULL;
  uint8_t data[READ_BYTES_AMOUNT];

  size_t n = 1;
  size_t total_bytes = 0;
  char json_str[READ_BYTES_AMOUNT];

  cmd_errno = CMD_SUCCESS;

  /* Read 8k at a time. */
  while (n != 0) {
    total_bytes += n;
    n = bufferevent_read(bev, data, sizeof(data));
  }

  snprintf(json_str, total_bytes, "%s", data);

  /* If the game is running, parse command list object */
  if (billionaire_game->running) {
    json_object* cmd_array = parse_command_list_string(json_str, total_bytes);

    if (cmd_errno != CMD_SUCCESS) {
      enqueue_command(this_client, billionaire_error());
    }

    else {
      JSON_ARRAY_FOREACH(cmd_obj, cmd_array) {
        /* Check cmd_object has command field */
        if (get_JSON_value(cmd_obj, "command") == NULL) {
          cmd_errno = (int) EBADCMDOBJ;
          enqueue_command(this_client, billionaire_error());
          continue;
        }

        if (command_is(cmd_obj, Command.NEW_OFFER)) {
          printf("Received NEW_OFFER from %s\n", this_client->id);

          /* Parse offer */
          json_object* card_array = get_JSON_value(cmd_obj, "cards");

          if (cmd_errno != CMD_SUCCESS) {
            enqueue_command(this_client, billionaire_error());
            continue;
          }

          card_location* card_loc = card_location_from_JSON(card_array);

          if (cmd_errno != CMD_SUCCESS) {
            enqueue_command(this_client, billionaire_error());
            continue;
          }

          /* Validate the offer */
          validate_offer(card_loc, this_client->hand);

          if (cmd_errno != CMD_SUCCESS) {
            if (cmd_errno == ESMALLOFFER) {
              /* Send CANCELLED_OFFER back to this_client */
              offer* bad_offer = offer_init(card_loc, this_client->id);

              json_object* cancel = billionaire_cancelled_offer(bad_offer);
              enqueue_command(this_client, cancel);

              free_offer(bad_offer);
            }

            enqueue_command(this_client, billionaire_error());
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
            enqueue_command(this_client, billionaire_error());
            free_offer(new_offer);
            continue;
          }

          /* Update this_client's hand */
          subtract_card_location(this_client->hand, new_offer->cards);

          if (cmd_errno != CMD_SUCCESS) {
            enqueue_command(this_client, billionaire_error());
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
            json_object* this_trade = billionaire_successful_trade(traded_offer);
            json_object* other_trade = billionaire_successful_trade(new_offer);

            free_offer(new_offer);
            free_offer(traded_offer);

            enqueue_command(this_client, this_trade);
            enqueue_command(other_client, other_trade);

            /* Send BOOK_EVENT to remaining players */
            const char* participants[MAX_PARTICIPANTS] = {this_client->id, other_client->id};
            json_object* book_event = billionaire_book_event(Command.SUCCESSFUL_TRADE,
                                                             total_cards,
                                                             participants);

            /*Check for win conditions */
            bool this_client_has_won = has_won(this_client->hand);
            bool other_client_has_won = has_won(other_client->hand);

            TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
              if (this_client_has_won) {
                enqueue_command(client_obj, billionaire_billionaire(this_client->id));
              }

              if (other_client_has_won) {
                enqueue_command(client_obj, billionaire_billionaire(other_client->id));
              }

              if (client_eq(client_obj, this_client) ||
                  client_eq(client_obj, other_client)) {
                continue;
              }

              enqueue_command(client_obj, book_event);
            }

            /* TODO: Reset the game */
            if (this_client_has_won || other_client_has_won) {
              clear_book(billionaire_game->current_trades);
            }
          }

          else {
            printf("Offer added to book\n");

            /* Send BOOK_EVENT to remaining players */
            const char* participants[MAX_PARTICIPANTS] = {this_client->id, NULL};
            json_object* book_event = billionaire_book_event(Command.NEW_OFFER,
                                                             total_cards,
                                                             participants);

            TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
              if (client_eq(client_obj, this_client)) {
                continue;
              }

              enqueue_command(client_obj, book_event);
            }
          }
        } /* Command.NEW_OFFER */

        else if (command_is(cmd_obj, Command.CANCEL_OFFER)) {
          printf("Received CANCEL_OFFER from %s\n", this_client->id);

          /* Parse offer */
          json_object* card_amt_json = get_JSON_value(cmd_obj, "card_amt");

          if (cmd_errno != CMD_SUCCESS) {
            enqueue_command(this_client, billionaire_error());
            continue;
          }

          size_t card_amt = (size_t) json_object_get_int(card_amt_json);

          offer* cancelled_offer = cancel_offer(billionaire_game->current_trades,
                                                card_amt, this_client->id);

          if (cmd_errno != CMD_SUCCESS) {
            enqueue_command(this_client, billionaire_error());
            continue;
          }

          /* Offer has been successfully cancelled */
          json_object* cancel = billionaire_cancelled_offer(cancelled_offer);
          enqueue_command(this_client, cancel);

          /* Send BOOK_EVENT to remaining players */
          const char* participants[MAX_PARTICIPANTS] = {this_client->id, NULL};
          json_object* book_event = billionaire_book_event(Command.CANCELLED_OFFER,
                                                           card_amt,
                                                           participants);

          TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
            if (client_eq(client_obj, this_client)) {
              continue;
            }

            enqueue_command(client_obj, book_event);
          }
        } /* Command.CANCEL_OFFER */

        else {
          /* Invalid command name */
          cmd_errno = (int) EBADCMDNAME;
          enqueue_command(this_client, billionaire_error());
          continue;
        }
      }
    }

    /* Free cmd_array after use */
    json_object_put(cmd_array);

    /* Send any outstanding commands to clients */
    send_commands_to_clients(&client_tailq_head);
  }

  else {
    /* This can eventually be removed */
    printf("Received from %s: %s\n", this_client->id, json_str);
  }
}

void
buffered_on_error(struct bufferevent* bev, short what, void* arg)
{
  client* this_client = (client*) arg;
  client* client_obj = NULL;

  if (what & BEV_EVENT_EOF) {
    /* Client disconnected, remove the read event and then
     * free the client structure. */
    printf("Client '%s' disconnected.\n", this_client->id);
  }
  else {
    warn("Client '%s' socket error, disconnecting.\n", this_client->id);
  }

  /* Remove the client from the tailq. */
  TAILQ_REMOVE(&client_tailq_head, this_client, entries);
  billionaire_game->num_players--;

  /* Remove the client from the hash table */
  del_client(hashed_clients, this_client);

  free_client(this_client);

  if (billionaire_game->running && (billionaire_game->num_players < billionaire_game->player_limit)) {
    printf("Player limit of %d no longer satisfied. Game stopping...\n",
           billionaire_game->player_limit);
    billionaire_game->running = false;

    /* Clear current trade book of current trades */
    clear_book(billionaire_game->current_trades);

    /* Send a FINISH command to each remaining client */
    json_object* finish = billionaire_finish();

    TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
      enqueue_command(client_obj, finish);
    }
  }

  send_commands_to_clients(&client_tailq_head);
}

void
on_accept(int fd, short ev, void* arg)
{
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(struct sockaddr_in);
  client* new_client;
  client* client_obj = NULL;

  char client_addr_str[ADDR_STR_SIZE];
  json_object* join;

  /* If game is running, deny connection (TODO) */
  if (billionaire_game->running) {
    return;
  }

  client_fd = accept(fd, (struct sockaddr*) &client_addr, &client_len);
  if (client_fd < 0) {
    warn("accept failed");
    return;
  }

  /* Set the client socket to non-blocking mode. */
  if (setnonblock(client_fd) < 0)
    warn("failed to set client socket non-blocking");

  /* We've accepted a new client, create a client object. */
  new_client = client_new(evbase, client_fd,
                          buffered_on_read, buffered_on_error);

  billionaire_game->num_players++;

  /* Get client address:port as a string */
  snprintf(client_addr_str, ADDR_STR_SIZE, "%s:%d",
           inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

  /* Create unique id from address:port */
  new_client->id = hash_addr(client_addr_str);

  /* Add client to client hash table */
  put_client(hashed_clients, new_client);

  printf("Accepted connection from %s (%s)\n", client_addr_str, new_client->id);

  /* Queue a JOIN command for the client. */
  join = billionaire_join(new_client->id);
  enqueue_command(new_client, join);

  /* Start game if max number of players has joined */
  if (billionaire_game->num_players >= billionaire_game->player_limit) {
    printf("Player limit of %d reached. Game starting...\n",
           billionaire_game->player_limit);
    billionaire_game->running = true;

    card_location** player_hands;

    printf("Dealing cards...\n");
    player_hands = deal_cards(billionaire_game->num_players,
                              billionaire_game->deck);

    size_t iplayer = 0;

    TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
      // 1) split up the deck between all players
      // 2) send each player their hand through the start command
      json_object* start = billionaire_start(player_hands[iplayer]);
      client_obj->hand = player_hands[iplayer];

      enqueue_command(client_obj, start);
      iplayer++;
    }

    free(player_hands);
  }

  /* Flush all client command queues to the corresponding client */
  send_commands_to_clients(&client_tailq_head);
}

void
on_sigint(int sig, short ev, void *arg)
{
  printf("\n");
  printf("SIGINT caught, exiting cleanly...\n");
  event_base_loopbreak(evbase);
}

void
parse_command_line_options(int argc, char** argv, int* player_limit,
                           bool* has_billionaire, bool* has_taxman) {
  while (true) {
    static struct option long_options[] = {
      {"players",        required_argument, 0, 'p'},
      {"no-billionaire", no_argument,       0, 'b'},
      {"no-taxman",      no_argument,       0, 't'},
      {"help",           no_argument,       0, 'h'}
    };

    int option_index = 0;

    int c = getopt_long(argc, argv, "p:bth", long_options, &option_index);

    /* End of options has been reached */
    if (c == -1)
      break;

    switch (c) {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        printf ("option %s", long_options[option_index].name);
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;

      case 'p':
        *player_limit = (int) strtol(optarg, NULL, 10);
        if (*player_limit > MAX_PLAYERS) {
          err(1, "player limit greater than %d", MAX_PLAYERS);
        }
        break;

      case 'b':
        *has_billionaire = false;
        break;

      case 't':
        *has_taxman = false;
        break;

      case 'h':
        printf("billionaire-server: a low-level TCP server for the Billionaire game\n");
        printf("\n");
        printf("  -p,--players N\tSet number of players (default: 4)\n");
        printf("  -b,--no-billionaire\tRemove billionaire from play\n");
        printf("  -t,--no-taxman\tRemove taxman from play\n");
        printf("  -h,--help\t\tDisplay this help and quit\n");
        exit(1);

      default:
        exit(1);
    }
  }
}

int
main(int argc, char** argv)
{
  /* Default parameters */
  int player_limit = 4;
  bool has_billionaire = true, has_taxman = true;

  int listen_fd;
  struct sockaddr_in listen_addr;
  struct event ev_accept, ev_sigint;

  /* Parse external options */
  parse_command_line_options(argc, argv,
                             &player_limit,
                             &has_billionaire, &has_taxman);

  // event_enable_debug_logging(EVENT_DBG_ALL);
  printf("Initialising server... ");

  /* Initialise libevent. */
  evbase = event_base_new();

  /* Initialise Billionaire game state */
  billionaire_game = game_state_new(player_limit,
                                    has_billionaire, has_taxman);

  /* Initialise client hash table */
  hashed_clients = client_hash_table_new(CLIENT_HASH_TABLE_SIZE);

  /* Initialise the tailq. */
  TAILQ_INIT(&client_tailq_head);

  /* Create our listening socket. */
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0)
    err(1, "listen failed");

  /* Allow socket address to be reused in case of crash/hard exit */
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

  memset(&listen_addr, 0, sizeof(struct sockaddr_in));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  listen_addr.sin_port = htons(SERVER_PORT);
  if (bind(listen_fd, (struct sockaddr*) &listen_addr,
           sizeof(struct sockaddr_in)) < 0) {
    printf("\n");
    err(1, "bind failed");
  }
  if (listen(listen_fd, 5) < 0) {
    printf("\n");
    err(1, "listen failed");
  }
  /* Set the socket to non-blocking, this is essential in event
   * based programming with libevent. */
  if (setnonblock(listen_fd) < 0)
    err(1, "failed to set server socket to non-blocking");

  printf("done\n");

  /* We now have a listening socket, we create a read event to
   * be notified when a client connects. */
  event_assign(&ev_accept, evbase, listen_fd, EV_READ|EV_PERSIST, on_accept, NULL);
  event_add(&ev_accept, NULL);

  /* Add SIGINT handling */
  evsignal_assign(&ev_sigint, evbase, SIGINT, on_sigint, NULL);
  event_add(&ev_sigint, NULL);

  /* Start the event loop. */
  event_base_dispatch(evbase);

  /* Called on SIGINT, or whenever the main event loop finishes */
  game_state_free(billionaire_game);

  /* Assumes that if client queue is empty, so is client hash table */
  if (!TAILQ_EMPTY(&client_tailq_head)) {
    struct client* client_struct;
    struct client* next_client_struct;

    client_struct = TAILQ_FIRST(&client_tailq_head);

    while (client_struct != NULL) {
      next_client_struct = TAILQ_NEXT(client_struct, entries);

      TAILQ_REMOVE(&client_tailq_head, client_struct, entries);
      del_client(hashed_clients, client_struct);
      free_client(client_struct);

      client_struct = next_client_struct;
    }
  }

  free_client_hash_table(hashed_clients);
  event_base_free(evbase);

  return 0;
}

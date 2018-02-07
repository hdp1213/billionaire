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
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <getopt.h>

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

          size_t total_cards = get_total_cards(card_loc);

          if (total_cards == 0) {
            printf("No cards in the offer. Ignoring...\n");
            continue;
          }

          else if (total_cards < 2) {
            /* Send CANCELLED_OFFER back to this_client */
            printf("Not enough cards in the offer, sent back...\n");
            continue;
          }

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

          offer* new_offer = offer_init(card_loc, this_client->id);
          offer* traded_offer = fill_offer(billionaire_game->current_trades,
                                           new_offer);

          if (cmd_errno != CMD_SUCCESS) {
            enqueue_command(this_client, billionaire_error());
            continue;
          }

          if (traded_offer != NULL) {
            /* A trade has been made */
            const char* other_id = traded_offer->owner_id;
            // client* other_client;

            json_object* this_trade = billionaire_successful_trade(traded_offer);
            json_object* other_trade = billionaire_successful_trade(new_offer);

            enqueue_command(this_client, this_trade);
            assert(other_trade != NULL);
            // enqueue_command(other_client, other_trade);

            /* Send BOOK_EVENT to remaining players */
            const char* participants[MAX_PARTICIPANTS] = {this_client->id, other_id};
            json_object* book_event = billionaire_book_event(Command.SUCCESSFUL_TRADE,
                                                             total_cards,
                                                             participants);

            TAILQ_FOREACH(client_obj, &client_tailq_head, entries) {
              if (client_eq(client_obj, this_client) ||
                  strncmp(client_obj->id, other_id, HASH_LENGTH) == 0) {
                continue;
              }

              enqueue_command(client_obj, book_event);
            }
          }

          else {
            printf("Offer added to book\n");

            /* Send BOOK_EVENT to remaining players */
            const char* participants[MAX_PARTICIPANTS] = {this_client->id, NULL};
            json_object* book_event = billionaire_book_event(Command.SUCCESSFUL_TRADE,
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

  bufferevent_free(this_client->buf_ev);
  close(this_client->fd);
  free(this_client->id);
  free(this_client);

  if (billionaire_game->running && (billionaire_game->num_players < billionaire_game->player_limit)) {
    printf("Player limit of %d no longer satisfied. Game stopping...\n",
           billionaire_game->player_limit);
    billionaire_game->running = false;

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
  socklen_t client_len = sizeof(client_addr);
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
      free_card_location(player_hands[iplayer]);

      enqueue_command(client_obj, start);
      iplayer++;
    }
  }

  /* Flush all client command queues to the corresponding client */
  send_commands_to_clients(&client_tailq_head);
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
  struct event ev_accept;
  int reuseaddr_on;

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

  /* Initialise the tailq. */
  TAILQ_INIT(&client_tailq_head);

  /* Create our listening socket. */
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0)
    err(1, "listen failed");
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  listen_addr.sin_port = htons(SERVER_PORT);
  if (bind(listen_fd, (struct sockaddr*) &listen_addr,
           sizeof(listen_addr)) < 0) {
    printf("\n");
    err(1, "bind failed");
  }
  if (listen(listen_fd, 5) < 0) {
    printf("\n");
    err(1, "listen failed");
  }
  reuseaddr_on = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
             sizeof(reuseaddr_on));

  /* Set the socket to non-blocking, this is essential in event
   * based programming with libevent. */
  if (setnonblock(listen_fd) < 0)
    err(1, "failed to set server socket to non-blocking");

  printf("done\n");

  /* We now have a listening socket, we create a read event to
   * be notified when a client connects. */
  event_assign(&ev_accept, evbase, listen_fd, EV_READ|EV_PERSIST,
               on_accept, NULL);
  event_add(&ev_accept, NULL);

  /* Start the event loop. */
  event_base_dispatch(evbase);

  /* Hopefully this is called, but probably not haha */
  // game_state_free(billionaire_game);

  return 0;
}

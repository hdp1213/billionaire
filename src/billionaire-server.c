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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <getopt.h>

#include "billionaire-server.h"
#include "utils.h"

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
  struct client* this_client = (struct client*) arg;
  struct client* client;
  uint8_t data[8192];
  size_t n;

  printf("Received from %s: ", this_client->id);

  /* Read 8k at a time and send it to all connected clients. */
  for (;;) {
    n = bufferevent_read(bev, data, sizeof(data));
    if (n <= 0) {
      /* Done. */
      break;
    }
    
    /* Send data to all connected clients except for the
     * client that sent the data. */
    /*
    TAILQ_FOREACH(client, &client_tailq_head, entries) {
      if (client != this_client) {
        bufferevent_write(client->buf_ev, data, n);
      }
    }
    */

    printf("%s", data);
  }
  printf("\n");
}

void
buffered_on_error(struct bufferevent* bev, short what, void* arg)
{
  struct client* client = (struct client*) arg;

  if (what & BEV_EVENT_EOF) {
    /* Client disconnected, remove the read event and the
     * free the client structure. */
    printf("Client '%s' disconnected.\n", client->id);
  }
  else {
    warn("Client '%s' socket error, disconnecting.\n", client->id);
  }

  /* Remove the client from the tailq. */
  TAILQ_REMOVE(&client_tailq_head, client, entries);
  billionaire_game->num_players--;

  bufferevent_free(client->buf_ev);
  close(client->fd);
  free(client->id);
  free(client);

  if (billionaire_game->running && (billionaire_game->num_players < billionaire_game->player_limit)) {
    printf("Player limit of %d no longer satisfied. Game stopping...\n",
           billionaire_game->player_limit);
    billionaire_game->running = false;

    /* Send a FINISH command to each remaining client */
    json_object* finish = billionaire_finish();

    TAILQ_FOREACH(client, &client_tailq_head, entries) {
      printf("Queued FINISH for %s\n", client->id);
      enqueue_command(client, finish);
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
  struct client* client;

  char client_addr_str[ADDR_STR_SIZE];
  json_object* join;
  json_object* start;

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
  client = calloc(1, sizeof(*client));
  if (client == NULL)
    err(1, "malloc failed");
  client->fd = client_fd;

  client->buf_ev = bufferevent_socket_new(evbase, client_fd, 0);
  bufferevent_setcb(client->buf_ev, buffered_on_read, NULL,
                    buffered_on_error, client);

  /* We have to enable it before our callbacks will be
   * called. */
  bufferevent_enable(client->buf_ev, EV_READ);

  /* Also initialise the command queue */
  STAILQ_INIT(&client->command_stailq_head);

  /* Add the new client to the tailq. */
  TAILQ_INSERT_TAIL(&client_tailq_head, client, entries);
  billionaire_game->num_players++;

  /* Get client address:port as a string */
  snprintf(client_addr_str, ADDR_STR_SIZE, "%s:%d",
           inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

  /* Create unique id from address:port */
  client->id = hash_addr(client_addr_str, ADDR_STR_SIZE);

  printf("Accepted connection from %s (%s)\n", client_addr_str, client->id);

  /* Queue a JOIN command for the client. */
  join = billionaire_join(client->id);
  enqueue_command(client, join);

  printf("Queued JOIN for %s\n", client->id);

  /* Start game if max number of players has joined */
  if (billionaire_game->num_players >= billionaire_game->player_limit) {
    printf("Player limit of %d reached. Game starting...\n",
           billionaire_game->player_limit);
    billionaire_game->running = true;

    card*** player_hands;
    size_t* player_hand_sizes;

    printf("Dealing cards...\n");
    deal_cards(billionaire_game->num_players, billionaire_game->deck,
               billionaire_game->deck_size, &player_hands,
               &player_hand_sizes);

    size_t iplayer = 0;

    TAILQ_FOREACH(client, &client_tailq_head, entries) {
      // 1) split up the deck between all players
      // 2) send each player their hand through the start command
      start = billionaire_start(player_hands[iplayer],
                                player_hand_sizes[iplayer]);

      printf("Queued START for %s\n", client->id);

      enqueue_command(client, start);
      iplayer++;
    }

    free(player_hand_sizes);
    free_player_hands(player_hands, billionaire_game->num_players);
  }

  /* Flush all client command queues to the corresponding client */
  send_commands_to_clients(&client_tailq_head);
}

void
enqueue_command(struct client* client, json_object* cmd)
{
  struct command* cmd_struct = calloc(1, sizeof(struct command));

  cmd_struct->cmd_json = cmd;
  STAILQ_INSERT_TAIL(&client->command_stailq_head, cmd_struct, cmds);
}

void
send_commands_to_clients(struct client_head* client_head)
{
  struct client* client;

  /* For each joined client, flush their command queue */
  TAILQ_FOREACH(client, client_head, entries) {
    struct command* cmd_struct;
    struct command* next_cmd_struct;

    json_object* command_wrapper = json_object_new_object();
    json_object* json_commands = json_object_new_array();

    /* If a client does not have any commands to flush, skip it */
    if (STAILQ_EMPTY(&client->command_stailq_head)) {
      continue;
    }

    /* This loop does not free memory allocated to the JSON object
       representing the actual command */
    cmd_struct = STAILQ_FIRST(&client->command_stailq_head);

    while (cmd_struct != NULL) {
      next_cmd_struct = STAILQ_NEXT(cmd_struct, cmds);
      json_object_array_add(json_commands, cmd_struct->cmd_json);

      // free(cmd_struct->cmd_json);
      free(cmd_struct);
      cmd_struct = next_cmd_struct;
    }

    /* Re-initialise the queue after it has been cleared */
    STAILQ_INIT(&client->command_stailq_head);

    /* Add JSON array to the wrapper object */
    json_object_object_add(command_wrapper, "commands", json_commands);

    /* Write resulting object to client's bufferevent */
    size_t cmd_len = 0;
    const char* cmd_str = JSON_to_str(command_wrapper, &cmd_len);
    bufferevent_write(client->buf_ev, cmd_str, cmd_len);

    printf("Sent queued command(s) to %s\n", client->id);

    free((void*) cmd_str);
    free(json_commands);
    free(command_wrapper);
  }
}

void
parse_command_line_options(int argc, char** argv, int* player_limit,
                           bool* has_billionaire, bool* has_taxman) {
  int c;

  while (true) {
    static struct option long_options[] = {
      {"players",        required_argument, 0, 'p'},
      {"no-billionaire", no_argument,       0, 'b'},
      {"no-taxman",      no_argument,       0, 't'},
      {"help",           no_argument,       0, 'h'}
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "p:bth", long_options, &option_index);

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

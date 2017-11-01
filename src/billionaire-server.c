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

  /* Read 8k at a time and send it to all connected clients. */
  for (;;) {
    n = bufferevent_read(bev, data, sizeof(data));
    if (n <= 0) {
      /* Done. */
      break;
    }
    
    /* Send data to all connected clients except for the
     * client that sent the data. */
    TAILQ_FOREACH(client, &client_tailq_head, entries) {
      if (client != this_client) {
        bufferevent_write(client->buf_ev, data, n);
      }
    }
  }
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
}

void
on_accept(int fd, short ev, void* arg)
{
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  struct client* client;

  char client_addr_str[ADDR_STR_SIZE];
  struct json_object* join;
  struct json_object* start;

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

  /* Add the new client to the tailq. */
  TAILQ_INSERT_TAIL(&client_tailq_head, client, entries);
  billionaire_game->num_players++;

  /* Get client address:port as a string */
  snprintf(client_addr_str, ADDR_STR_SIZE, "%s:%d",
           inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

  printf("Accepted connection from %s\n", client_addr_str);

  /* Send a JOIN command to the client. */
  join = billionaire_join(client_addr_str, ADDR_STR_SIZE, &client->id);
  send_command(client->buf_ev, join);

  printf("Sent JOIN to %s (%s)\n", client_addr_str, client->id);

  /* Free JOIN command */
  free(join);

  /* Start game if max number of players has joined */
  if (billionaire_game->num_players >= billionaire_game->player_limit) {
    printf("Starting game...\n");
    billionaire_game->running = true;

    TAILQ_FOREACH(client, &client_tailq_head, entries) {
      // 1) split up the deck between all players
      // 2) send each player their hand through the start command
      // start = billionaire_start();

      printf("Sent START to %s\n", client->id);
    }
  }
}

void
send_command(struct bufferevent* bev, struct json_object* cmd)
{
  size_t cmd_len = 0;
  const char* cmd_str = JSON_to_str(cmd, &cmd_len);
  bufferevent_write(bev, cmd_str, cmd_len);

  free((void*) cmd_str);
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

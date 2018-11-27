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

#include "server.h"

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
#include <time.h> /* clock(), time() */
#include <unistd.h> /* getpid() */

#include "billionaire.h"
#include "client.h"
#include "client_hash_table.h"
#include "command.h"
#include "game_state.h"
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
  uint8_t data[READ_BYTES_AMOUNT];

  size_t n = 1;
  size_t total_bytes = 0;
  char json_str[READ_BYTES_AMOUNT];

  /* Read 8k at a time. */
  while (n != 0) {
    total_bytes += n;
    n = bufferevent_read(bev, data, sizeof(data));
  }

  snprintf(json_str, total_bytes, "%s", data);

  /* If the game is running, parse command list object */
  if (is_running(billionaire_game)) {
    process_client_command(this_client, json_str, total_bytes);
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

  if (is_running(billionaire_game) && !is_full(billionaire_game)) {
    stop_billionaire_game();
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

  char client_addr_str[ADDR_STR_SIZE];
  json_object* join;

  /* If game is running, deny connection (TODO) */
  if (is_running(billionaire_game)) {
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
  join = command_join(new_client->id);
  enqueue_command(new_client, join);

  /* Start game if max number of players has joined */
  if (is_full(billionaire_game)) {
    start_billionaire_game();
  }

  /* Flush all client command queues to the corresponding client */
  send_commands_to_clients(&client_tailq_head);
}

void
on_exit(int sig, short ev, void *arg)
{
  printf("\n");
  printf("Exiting cleanly...\n");
  event_base_loopbreak(evbase);
}

void
parse_command_line_options(int argc, char** argv, int* player_limit,
                           bool* has_billionaire, bool* has_taxman,
                           uint32_t* seed) {
  while (true) {
    static struct option long_options[] = {
      {"players",        required_argument, 0, 'p'},
      {"no-billionaire", no_argument,       0, 'b'},
      {"no-taxman",      no_argument,       0, 't'},
      {"seed",           required_argument, 0, 's'},
      {"help",           no_argument,       0, 'h'}
    };

    int option_index = 0;

    int c = getopt_long(argc, argv, "p:bts:h", long_options, &option_index);

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

      case 's':
        *seed = (uint32_t) strtol(optarg, NULL, 10);
        break;

      case 'h':
        printf("billionaire-server: a low-level TCP server for the Billionaire game\n");
        printf("\n");
        printf("  -p,--players N\tSet number of players (default: 4)\n");
        printf("  -b,--no-billionaire\tRemove billionaire from play\n");
        printf("  -t,--no-taxman\tRemove taxman from play\n");
        printf("  -s,--seed N\t\tSet the random seed (default: random)\n");
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
  uint32_t seed = mix(clock(), time(NULL), getpid());

  int listen_fd;
  struct sockaddr_in listen_addr;
  struct event ev_accept, ev_sigint, ev_sigterm;

  /* Parse external options */
  parse_command_line_options(argc, argv,
                             &player_limit,
                             &has_billionaire, &has_taxman,
                             &seed);

  srand(seed);

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

  /* Add SIGINT and SIGTERM handling */
  evsignal_assign(&ev_sigint, evbase, SIGINT, on_exit, NULL);
  evsignal_assign(&ev_sigterm, evbase, SIGTERM, on_exit, NULL);
  event_add(&ev_sigint, NULL);
  event_add(&ev_sigterm, NULL);

  /* Start the main event loop */
  event_base_dispatch(evbase);

  /* Called whenever the main event loop finishes */
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

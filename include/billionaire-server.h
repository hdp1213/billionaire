#ifndef _BILLIONAIRE_SERVER_H_
#define _BILLIONAIRE_SERVER_H_

/* Required by event.h. */
#include <sys/time.h>

#include <stdlib.h>

#include <sys/queue.h>

/* Libevent. */
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

/* JSON */
#include <json-c/json.h>

#include "billionaire.h"
#include "game_state.h"

/* Port to listen on. */
#define SERVER_PORT 5555

/* Maximum size of address string used for hashing */
#define ADDR_STR_SIZE 23

/**
 * External command struct used for checking command types.
 */
extern const struct commands Command;

/* The libevent event base.  In libevent 1 you didn't need to worry
 * about this for simple programs, but its used more in the libevent 2
 * API. */
static struct event_base* evbase;

/**
 * Global game state structure.
 */
static game_state* billionaire_game;

/**
 * Command structure used as a queue entry.
 */
struct command {
  json_object* cmd_json;

  STAILQ_ENTRY(command) cmds;
};

/**
 * A struct for client specific data.
 *
 * This also includes the tailq entry item so this struct can become a
 * member of a tailq - the linked list of all connected clients.
 */
struct client {
  /* The clients socket. */
  int fd;

  /* The client ID. */
  char* id;

  /* The bufferedevent for this client. */
  struct bufferevent* buf_ev;

  /*
   * This holds the pointers to the next and previous entries in
   * the tail queue.
   */
  TAILQ_ENTRY(client) entries;

  /* The head of the single tail queue for commands */
  STAILQ_HEAD(, command) command_stailq_head;
};

/**
 * The head of our tailq of all connected clients.  This is what will
 * be iterated to send a received message to all connected clients.
 */
TAILQ_HEAD(client_head, client) client_tailq_head;

/**
 * Set a socket to non-blocking mode.
 */
int setnonblock(int fd);

/**
 * Called by libevent when there is data to read.
 *
 * This is where all of the server logic should go.
 */
void buffered_on_read(struct bufferevent* bev, void* arg);

/**
 * Called by libevent when there is an error on the underlying socket
 * descriptor.
 */
void buffered_on_error(struct bufferevent* bev, short what, void* arg);

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(int fd, short ev, void* arg);

/**
 * Add a Billionaire command to the client's command queue.
 */
void enqueue_command(struct client* client, json_object* cmd);

/**
 * Send a series of Billionaire commands to each client.
 *
 * Runs through the command STAILQ head, popping command structures off
 * and creating a json_object that is then sent to the corresponding
 * client.
 *
 * Memory allocated to the JSON objects is (hopefully) freed here.
 */
void send_commands_to_clients(struct client_head* client_head);

/**
 * Handle command line options using getopt_long.
 */
void parse_command_line_options(int argc, char** argv,
                                int* player_limit,
                                bool* has_billionaire,
                                bool* has_taxman);

#endif

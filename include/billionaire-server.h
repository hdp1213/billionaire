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

#include "billionaire.h"
#include "game_state.h"
#include "client_hash_table.h"

/* Port to listen on. */
#define SERVER_PORT 5555

/* Maximum size of address string used for hashing */
#define ADDR_STR_SIZE 23

/* Client hash table size */
#define CLIENT_HASH_TABLE_SIZE 64

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
 * The static hash table containing all hashed clients.
 */
static client_hash_table* hashed_clients;

/**
 * Set a socket to non-blocking mode.
 */
int setnonblock(int fd);

/**
 * Called by libevent when there is data to read.
 *
 * This is where all of the server logic should go.
 * Responds to changes in cmd_errno by sending ERROR commands.
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
 * Handle command line options using getopt_long.
 */
void parse_command_line_options(int argc, char** argv,
                                int* player_limit,
                                bool* has_billionaire,
                                bool* has_taxman);

#endif

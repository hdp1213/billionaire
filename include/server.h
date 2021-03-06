#ifndef _SERVER_H_
#define _SERVER_H_

/* Required by event.h. */
#include <sys/time.h>

#include <stdbool.h>
#include <stdlib.h>

#include <sys/queue.h>

/* Libevent. */
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

/* Port to listen on. */
#define SERVER_PORT 5555

/* Maximum size of address string used for hashing */
#define ADDR_STR_SIZE 23

/* Client hash table size */
#define CLIENT_HASH_TABLE_SIZE 64

/* The libevent event base.  In libevent 1 you didn't need to worry
 * about this for simple programs, but its used more in the libevent 2
 * API. */
static struct event_base* evbase;

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
 * Called by libevent when there is a connection ready to be accepted.
 */
void on_accept(int fd, short ev, void* arg);

/**
 * Called by libevent when a SIGINT or SIGTERM signal is caught.
 *
 * This breaks the base loop and allows cleanup code to execute.
 */
void on_exit(int sig, short ev, void *arg);

/**
 * Handle command line options using getopt_long.
 */
void parse_command_line_options(int argc, char** argv,
                                int* player_limit,
                                bool* has_billionaire,
                                bool* has_taxman,
                                uint32_t* seed);

#endif

#ifndef _CHAT_SERVER_H_
#define _CHAT_SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Required by event.h. */
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <sys/queue.h>

/* Libevent. */
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

/* Port to listen on. */
#define SERVER_PORT 5555

/* The libevent event base.  In libevent 1 you didn't need to worry
 * about this for simple programs, but its used more in the libevent 2
 * API. */
static struct event_base *evbase;

/**
 * A struct for client specific data.
 *
 * This also includes the tailq entry item so this struct can become a
 * member of a tailq - the linked list of all connected clients.
 */
struct client {
  /* The clients socket. */
  int fd;

  /* The bufferedevent for this client. */
  struct bufferevent *buf_ev;

  /*
   * This holds the pointers to the next and previous entries in
   * the tail queue.
   */
  TAILQ_ENTRY(client) entries;
};

/**
 * The head of our tailq of all connected clients.  This is what will
 * be iterated to send a received message to all connected clients.
 */
TAILQ_HEAD(, client) client_tailq_head;

/**
 * Set a socket to non-blocking mode.
 */
int setnonblock(int fd);

/**
 * Called by libevent when there is data to read.
 */
void buffered_on_read(struct bufferevent *bev, void *arg);

/**
 * Called by libevent when there is an error on the underlying socket
 * descriptor.
 */
void buffered_on_error(struct bufferevent *bev, short what, void *arg);

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(int fd, short ev, void *arg);




#endif

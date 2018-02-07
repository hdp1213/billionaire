#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdbool.h>
#include <sys/queue.h>

/* Libevent */
#include <event2/bufferevent.h>

/* JSON */
#include <json-c/json.h>

typedef struct client client;
typedef struct client_head client_head;

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
 * Command structure used as a queue entry.
 */
struct command {
  json_object* cmd_json;

  STAILQ_ENTRY(command) cmds;
};

/**
 * Add a Billionaire command to the client's command queue.
 */
void enqueue_command(client* client_obj, json_object* cmd);

/**
 * Send a series of Billionaire commands to each client.
 *
 * Runs through the command STAILQ head, popping command structures off
 * and creating a json_object that is then sent to the corresponding
 * client.
 *
 * Memory allocated to the JSON objects is (hopefully) freed here.
 */
void send_commands_to_clients(client_head* client_head_obj);

/**
 * Compare two clients for equality.
 *
 * Two clients are equal if their IDs are equal and their sockets are
 * the same.
 */
bool client_eq(client* client1, client* client2);

#endif

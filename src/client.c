#include "client.h"
#include "billionaire.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

void
enqueue_command(client* client_obj, json_object* cmd)
{
  struct command* cmd_struct = calloc(1, sizeof(struct command));

  size_t cmd_len;
  const char* cmd_str = get_command_name(cmd, &cmd_len);

  printf("Queued %s for %s\n", cmd_str, client_obj->id);

  cmd_struct->cmd_json = cmd;
  STAILQ_INSERT_TAIL(&client_obj->command_stailq_head, cmd_struct, cmds);
}

void
send_commands_to_clients(client_head* client_head_obj)
{
  client* client_obj = NULL;

  /* For each joined client, flush their command queue */
  TAILQ_FOREACH(client_obj, client_head_obj, entries) {
    struct command* cmd_struct;
    struct command* next_cmd_struct;

    json_object* command_wrapper = json_object_new_object();
    json_object* json_commands = json_object_new_array();

    /* If a client does not have any commands to flush, skip it */
    if (STAILQ_EMPTY(&client_obj->command_stailq_head)) {
      continue;
    }

    /* This loop does not free memory allocated to the JSON object
       representing the actual command */
    cmd_struct = STAILQ_FIRST(&client_obj->command_stailq_head);

    while (cmd_struct != NULL) {
      next_cmd_struct = STAILQ_NEXT(cmd_struct, cmds);
      json_object_array_add(json_commands, cmd_struct->cmd_json);

      free(cmd_struct);
      cmd_struct = next_cmd_struct;
    }

    /* Re-initialise the queue after it has been cleared */
    STAILQ_INIT(&client_obj->command_stailq_head);

    /* Add JSON array to the wrapper object */
    json_object_object_add(command_wrapper, "commands", json_commands);

    /* Write resulting object to client's bufferevent */
    size_t cmd_len = 0;
    const char* cmd_str = JSON_to_str(command_wrapper, &cmd_len);
    bufferevent_write(client_obj->buf_ev, cmd_str, cmd_len);

    printf("Sent queued command(s) to %s\n", client_obj->id);

    /* Free the command wrapper and its constituent objects */
    json_object_put(command_wrapper);
  }
}

bool
client_eq(struct client* client1, struct client* client2)
{
  return (strncmp(client1->id, client2->id, HASH_LENGTH) == 0) &&
         (client1->fd == client2->fd);
}

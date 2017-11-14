#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>

#include "billionaire.h"
#include "utils.h"

const struct commands Command = {
  "JOIN", "START", "RECEIVE", "BOOK_STATE", "CHECK", "FINISH", "ASK", "CANCEL", "BILLIONAIRE"
};

json_object*
make_command(const char* cmd_name)
{
  json_object* cmd = json_object_new_object();
  json_object* cmd_str = json_object_new_string(cmd_name);
  json_object_object_add(cmd, "command", cmd_str);

  return cmd;
}

json_object*
billionaire_join(char* id)
{
  json_object* cmd = make_command(Command.JOIN);

  json_object* bot_id = json_object_new_string(id);
  json_object_object_add(cmd, "bot_id", bot_id);

  return cmd;
}

json_object*
billionaire_start(card** player_cards, size_t num_cards)
{
  json_object* cmd = make_command(Command.START);

  json_object* hand = json_object_new_array();

  for (size_t i = 0; i < num_cards; ++i) {
    json_object* card = JSON_from_card(player_cards[i]);
    json_object_array_add(hand, card);
  }

  json_object_object_add(cmd, "hand", hand);

  return cmd;
}

json_object*
billionaire_finish()
{
  return make_command(Command.FINISH);
}

const char*
get_command_name(json_object* cmd, size_t* str_len)
{
  json_object* cmd_str_json = get_JSON_value(cmd, "command");

  const char* cmd_str = json_object_get_string(cmd_str_json);
  *str_len = (size_t) json_object_get_string_len(cmd_str_json);

  return cmd_str;
}

bool
command_is(json_object* cmd, const char* cmd_name)
{
  size_t cmd_len = 0;
  const char* cmd_str = get_command_name(cmd, &cmd_len);

  return (strcmp(cmd_str, cmd_name) == 0);
}

#include <stdio.h>
#include <string.h>

#include "billionaire.h"
#include "command_error.h"
#include "utils.h"

const struct commands Command = {
  "JOIN", "START", "SUCCESSFUL_TRADE", "CANCELLED_OFFER", "BOOK_EVENT",
  "BILLIONAIRE", "END_GAME", "ERROR", "NEW_OFFER", "CANCEL_OFFER"
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

  json_object* client_id = json_object_new_string(id);
  json_object_object_add(cmd, "client_id", client_id);

  return cmd;
}

json_object*
billionaire_start(card_location* player_hand)
{
  json_object* cmd = make_command(Command.START);

  json_object* hand_json = JSON_from_card_location(player_hand);
  json_object_object_add(cmd, "hand", hand_json);

  json_object* score_json = json_object_new_int(INITIAL_SCORE);
  json_object_object_add(cmd, "score", score_json);

  return cmd;
}

json_object*
billionaire_successful_trade(offer* traded_offer)
{
  json_object* cmd = make_command(Command.SUCCESSFUL_TRADE);

  json_object* offer_cards = JSON_from_card_location(traded_offer->cards);
  json_object* offer_owner = json_object_new_string(traded_offer->owner_id);

  json_object_object_add(cmd, "cards", offer_cards);
  json_object_object_add(cmd, "owner_id", offer_owner);

  return cmd;
}

json_object*
billionaire_cancelled_offer(offer* cancelled_offer)
{
  json_object* cmd = make_command(Command.CANCELLED_OFFER);

  json_object* offer_cards = JSON_from_card_location(cancelled_offer->cards);
  json_object_object_add(cmd, "cards", offer_cards);

  return cmd;
}

json_object*
billionaire_book_event(const char* event, size_t card_amt,
                       const char* participants[MAX_PARTICIPANTS])
{
  json_object* cmd = make_command(Command.BOOK_EVENT);

  json_object* event_json = json_object_new_string(event);
  json_object* card_amt_json = json_object_new_int((int) card_amt);
  json_object* participants_json = json_object_new_array();

  for (int i = 0; i < MAX_PARTICIPANTS; ++i) {
    const char* participant = participants[i];

    if (participant == NULL) {
      continue;
    }

    json_object* participant_json = json_object_new_string(participant);
    json_object_array_add(participants_json, participant_json);
  }

  json_object_object_add(cmd, "event", event_json);
  json_object_object_add(cmd, "card_amt", card_amt_json);
  json_object_object_add(cmd, "participants", participants_json);

  return cmd;
}

json_object*
billionaire_billionaire(const char* winner_id)
{
  json_object* cmd = make_command(Command.BILLIONAIRE);

  json_object* winner_json = json_object_new_string(winner_id);

  json_object_object_add(cmd, "winner_id", winner_json);

  return cmd;
}

json_object*
billionaire_end_game()
{
  return make_command(Command.END_GAME);
}

json_object*
billionaire_error()
{
  json_object* cmd = make_command(Command.ERROR);

  json_object* errno_json = json_object_new_int(cmd_errno);

  char* what;

  if (cmd_errno <= EJSON) { /* The error comes from <json-c/json-c.h> */
    what = (char*) json_tokener_error_desc(cmd_errno);
    printf("External JSON error, %s\n", what);
  }

  else { /* The error is internal and has a specified reason */
    what = (char*) error_what[cmd_errno - EJSON - 1];
    printf("Internal error, %d: %s\n", cmd_errno - EJSON - 1, what);
  }

  json_object* what_json = json_object_new_string(what);

  json_object_object_add(cmd, "errno", errno_json);
  json_object_object_add(cmd, "what", what_json);

  cmd_errno = CMD_SUCCESS;

  return cmd;
}

const char*
get_command_name(json_object* cmd, size_t* str_len)
{
  json_object* cmd_str_json = get_JSON_value(cmd, "command");

  if (cmd_errno != CMD_SUCCESS) {
    *str_len = 0;
    return NULL;
  }

  const char* cmd_str = json_object_get_string(cmd_str_json);
  *str_len = (size_t) json_object_get_string_len(cmd_str_json);

  return cmd_str;
}

json_object*
parse_command_list_string(char* json_str, size_t str_len)
{
  json_object* parse_obj = str_to_JSON(json_str, str_len);

  if (cmd_errno != CMD_SUCCESS) {
    json_object_put(parse_obj);
    return NULL;
  }

  json_object* cmd_array = get_JSON_value(parse_obj, "commands");

  if (cmd_errno != CMD_SUCCESS) {
    json_object_put(parse_obj);
    return NULL;
  }

  if (!json_object_is_type(cmd_array, json_type_array)) {
    cmd_errno = (int) EJSONTYPE;
    return NULL;
  }

  json_object_get(cmd_array);
  json_object_put(parse_obj);

  return cmd_array;
}

bool
command_is(json_object* cmd, const char* cmd_name)
{
  size_t cmd_len = 0;
  const char* cmd_str = get_command_name(cmd, &cmd_len);

  return (strcmp(cmd_str, cmd_name) == 0);
}

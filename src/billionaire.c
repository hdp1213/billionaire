#include <stdlib.h>
#include <stdio.h>

#include "billionaire.h"

json_object*
make_command(const char* cmd)
{
  json_object* res = json_object_new_object();
  json_object* cmd_str = json_object_new_string(cmd);
  json_object_object_add(res, "command", cmd_str);

  return res;
}

json_object*
billn_join()
{
  json_object* cmd = make_command("JOIN");

  json_object* bot_id = json_object_new_int(BOT_ID);
  json_object_object_add(cmd, "bot_id", bot_id);

  BOT_ID++;

  return cmd;
}

const char*
stringify_command(json_object* json_obj, size_t* str_len)
{
  return json_object_to_json_string_length(json_obj,
                                           JSON_C_TO_STRING_PLAIN,
                                           str_len);
}

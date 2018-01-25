#include "command_error.h"

int cmd_errno = CMD_SUCCESS;

const char* error_what[7] = {
  "JSON value unable to be extracted",
  "Invalid command name",
  "Command object does not contain 'command' field",
  "Offer to cancel is empty",
  "Offer could not be cancelled due to incorrect owner",
  "New offer overrides previously declared offer",
  "Bad JSON type"
};

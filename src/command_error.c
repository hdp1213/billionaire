#include "command_error.h"

int cmd_errno = CMD_SUCCESS;

const char* error_what[NUM_ERRORS] = {
  "JSON value unable to be extracted",
  "JSON object is not the desired type",
  "Invalid command name",
  "Command object does not contain 'command' field",
  "Offer to cancel is empty",
  "Offer to cancel not owned by client",
  "New offer overrides previously declared offer"
};

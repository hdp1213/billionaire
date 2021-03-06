#include "command_error.h"

int cmd_errno = CMD_SUCCESS;

const char* error_what[] = {
  "JSON value unable to be extracted",
  "JSON object is not the desired type",
  "Invalid command name",
  "Command object does not contain 'command' field",
  "Offer to cancel is empty",
  "Offer to cancel not owned by client",
  "New offer overrides previously declared offer",
  "Offer does not contain any cards",
  "Offer does not contain enough cards",
  "Cards in offer do not exist in hand",
  "Offer contains too many unique commodities",
  "Offer contains too many unique wildcards",
  "Not enough cards to remove from card_location"
};

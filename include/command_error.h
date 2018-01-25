#include <json-c/json.h>

#define MAX_ERROR_LENGTH 64
#define CMD_SUCCESS 0

/* Need to distinguish between fatal errors and non-fatal errors */

enum errorno {
  EJSON = json_tokener_error_size, /* Upper value for JSON parsing errors */
  EJSONVAL, /* JSON value unable to be extracted */
  EBADCMDNAME, /* Invalid command name */
  EBADCMDOBJ, /* Command object does not contain 'command' field */
  ECANEMPTY, /* Offer to cancel is empty */
  ECANPERM, /* Offer could not be cancelled due to permission error */
  EOFFEROVER, /* New offer overrides previously declared offer */
  EBADTYPE /* Bad JSON type */
};

extern int cmd_errno;

// extern const char error_what[4][MAX_ERROR_LENGTH];
extern const char* error_what[7];
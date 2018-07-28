#ifndef _BILLIONAIRE_H_
#define _BILLIONAIRE_H_

#include "client.h"

/**
 * Processes the raw JSON string sent by a client
 */
void process_client_command(client* this_client, char json_str[],
                            size_t str_size);

#endif

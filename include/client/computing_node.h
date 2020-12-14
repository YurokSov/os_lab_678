#ifndef INCLUDE_CUMPUTING_NODE_H
#define INCLUDE_CUMPUTING_NODE_H

#include "core/message_manager.h"

typedef enum node_status {
    ns_ok, ns_error,
} node_status;

node_status node_start();

node_status execute_command(mm_command* data);

#endif
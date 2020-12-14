#ifndef INCLUDE_CUMPUTING_NODE_H
#define INCLUDE_CUMPUTING_NODE_H

typedef enum node_status {
    ns_ok, ns_error,
} node_status;

node_status node_start();

#endif
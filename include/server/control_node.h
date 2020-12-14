#ifndef INCLUDE_CONTROL_NODE_H
#define INCLUDE_CONTROL_NODE_H

#include <stdlib.h>
#include <stdbool.h>

#include "../server/parsing.h"
#include "../core/defines.h"

#define MAX_NODES 256

typedef enum execute_status {
    es_ok, es_bad_params, es_runtime, es_node_error, es_msgq_error,
} execute_status;

typedef struct node_info {
    __pid_t pid;
    __pid_t ppid;
    DWORD id;
    DWORD p_id;
} node_info;

typedef struct node {
    bool is_alive;
    node_info info;
} node;

bool init_control_node();

bool deinit_control_node();

execute_status execute_cmd(cmd_enum* cmd, command_u* cmd_info, void** result);

void kill_childs();

#endif
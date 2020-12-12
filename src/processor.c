#include "processor.h"
#include "computing_node.h"

#include "logger.h"

static node nodes_table[MAX_NODES];

execute_status execute_create(create_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    __pid_t fork_res = fork();
    if (fork_res > 0) {
        LOG(LL_NOTE, "In Parent, created node");
        //create_add_hook(cmd_info, fork_res, result);
    }
    else if (fork_res == 0) {
        LOG(LL_NOTE, "In Child, fresh meat");
        if (node_start() != ns_ok)
            status = es_node_error;
        exit(EXIT_SUCCESS);
    }
    else {
        LOG(LL_ERROR, "Couldn\'t create computing node! ");
        perror("ERROR:");
        status = es_runtime;
    }
    return status;
}

execute_status execute_remove(remove_cmd* cmd_info, void** result) {

}

execute_status execute_exec(exec_cmd* cmd_info, void** result) {

}

execute_status execute_pingall(pingall_cmd* cmd_info, void** result) {

}

execute_status execute_cmd(cmd_enum* cmd, command_u* cmd_info, void** result) {
    execute_status status;

    switch (*cmd) {
    case ce_create:
        status = execute_create(&(cmd_info->create), result);
        break;
    case ce_remove:
        status = execute_remove(&(cmd_info->remove), result);
        break;
    case ce_exec:
        status = execute_exec(&(cmd_info->exec), result);
        break;
    case ce_pingall:
        status = execute_pingall(&(cmd_info->pingall), result);
        break;
    }
    if (status == es_ok) {
        LOG(LL_NOTE, "Execution status: OK");
    }
    else {
        LOG(LL_NOTE, "Execution status: ERROR");
    }

    return status;
}
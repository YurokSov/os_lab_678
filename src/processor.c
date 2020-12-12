#include "processor.h"
#include "computing_node.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

#include "logger.h"

static node ntable[MAX_NODES];
static int ntable_size = 0;

void create_add_hook(create_cmd* cmd_info, int fork_res, void** result) {
    ntable[ntable_size].is_alive = true;
    ntable[ntable_size].info.pid = fork_res;
    ntable[ntable_size].info.ppid = getpid();
    ntable[ntable_size].info.id = cmd_info->id;
    ntable[ntable_size].info.parent_id = cmd_info->parent_id;
    ntable_size++;
}

execute_status execute_create(create_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    __pid_t fork_res = fork();
    if (fork_res > 0) {
        LOG(LL_NOTE, "In Parent, created node");
        create_add_hook(cmd_info, fork_res, result);
    }
    else if (fork_res == 0) {
        // if (!_LOG_INIT())
        //    printf("log init error in pid:%d", getpid());
        LOG(LL_NOTE, "In Child, fresh meat");
        if (node_start() != ns_ok)
            status = es_node_error;
        //if (!_LOG_DEINIT())
        //    printf("log deinit error in pid:%d", getpid());
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

void kill_childs() {
    for (int i = 0; i < MAX_NODES; ++i) {
        if (ntable[i].is_alive) {
            if (kill(ntable[i].info.pid, SIGTERM) == ERROR_CODE) {
                LOG(LL_ERROR, "Error while killing child node!");
                perror("ERROR");
            }
        }
    }
    int status;
    pid_t wpid;
    for (int i = 0; i < MAX_NODES; ++i) {
        if (ntable[i].is_alive) {
            if (waitpid(ntable[i].info.pid, &status, 0) == ERROR_CODE) {
                LOG(LL_ERROR, "Error while waiting child node!");
                perror("ERROR");
            }
            else {
                ntable[i].is_alive = false;
            }
            if (WIFEXITED(status))
                LOG(LL_NOTE, "Child process exited successfully");
            else
                LOG(LL_WARNING, "Child process exited with error: %d", status);
        }
    }
    sleep(1);
    LOG(LL_NOTE, "All child processes are exited!");
}
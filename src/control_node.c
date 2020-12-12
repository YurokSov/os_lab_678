#include "control_node.h"
#include "computing_node.h"
#include "message_manager.h"
#include "logger.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

#include "avl_c_externs.h"

static struct avl_tree* tree_ptr;

static node ntable[MAX_NODES];
static int ntable_size = 0;

bool init_control_node() {
    for (int i = 0; i < MAX_NODES; ++i)
        ntable[i].is_alive = false;
    return init_avl(&tree_ptr);
}

bool deinit_control_node() {
    return deinit_avl(tree_ptr);
}

bool create_check_for_validity(create_cmd* cmd_info) {
    for (int i = 0; i < ntable_size; ++i) {
        if (ntable[i].info.id == cmd_info->id) {
            printf("Error: Already exists\n");
            return false;
        }
    }
    return true;
}

bool remove_check_for_validity(remove_cmd* cmd_info) {
    for (int i = 0; i < ntable_size; ++i)
        if (ntable[i].info.id == cmd_info->id)
            return true;
    return false;
}

bool exec_check_for_validity(exec_cmd* cmd_info) {
    for (int i = 0; i < ntable_size; ++i)
        if (ntable[i].info.id == cmd_info->id)
            return true;
    return false;
}

void create_add_hook(create_cmd* cmd_info, int fork_res) {
    if (!add_to_tree(tree_ptr, cmd_info->id)) {
        LOG(LL_FATAL, "BAD TREE INSERT");
        exit(EXIT_FAILURE);
    }
    ntable[ntable_size].is_alive = true;
    ntable[ntable_size].info.pid = fork_res;
    ntable[ntable_size].info.ppid = getpid();
    ntable[ntable_size].info.id = cmd_info->id;
    ntable[ntable_size].info.parent_id = get_parent_id(tree_ptr, cmd_info->id);
    ntable_size++;
}

execute_status execute_create(create_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    if (!create_check_for_validity(cmd_info)) {
        status = es_bad_params;
    }
    else {
        __pid_t fork_res = fork();
        if (fork_res > 0) {
            LOG(LL_NOTE, "In Parent, created child with id: %d", cmd_info->id);
            create_add_hook(cmd_info, fork_res);
            int path_len;
            int* path = get_path(tree_ptr, ntable[ntable_size - 1].info.id, &path_len);
            mm_send_creat(path, path_len);
            printf("Ok: %d", fork_res);
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
    }
    return status;
}

execute_status execute_remove(remove_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    if (!remove_check_for_validity(cmd_info)) {
        status = es_bad_params;
    }
    else {
        LOG(LL_NOTE, "Killing child with id: %d", cmd_info->id);
        bool found = false;
        for (int i = 0; i < ntable_size; ++i) {
            if (ntable[i].info.id == cmd_info->id) {
                found = true;
            }
        }
        if (!found) {
            printf("Error: Not found");
            LOG(LL_NOTE, "child not found");
        }
        else {
            int path_len;
            int* path = get_path(tree_ptr, cmd_info->id, &path_len);
            if (mm_send_remov(path, path_len) == mmr_ok)
                printf("Ok\n");
            else
                printf("Error: Node is unavailable\n");
        }
    }
    return status;
}

execute_status execute_exec(exec_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    if (!exec_check_for_validity(cmd_info)) {
        status = es_bad_params;
    }
    else {
        int path_len;
        int* path = get_path(tree_ptr, cmd_info->id, &path_len);
        search_pat_in_text cmd = { cmd_info->pattern_len, cmd_info->text_len, cmd_info->pattern, cmd_info->text };
        if (!mm_send_exec(cmd, path, path_len))
            status = es_msgq_error;
    }
    return status;
}

execute_status execute_pingall(pingall_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    for (int i = 0; i < ntable_size; ++i) {
        if (ntable[i].is_alive) {
            int path_len;
            int* path = get_path(tree_ptr, ntable[i].info.id, &path_len);
            if (mm_send_ping(path, path_len) != mmr_ok) {
                status = es_msgq_error;
            }
        }
    }
    return status;
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
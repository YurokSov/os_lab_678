#include "core/message_manager.h"
#include "server/control_node.h"
#include "server/mm_control_node.h"
#include "client/computing_node.h"
#include "client/mm_computing_node.h"
#include "utils/logger.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "utils/avl_c_externs.h"

static struct avl_tree* tree_ptr;

static node ntable[MAX_NODES];
static int ntable_size = 0;

bool init_control_node() {
    for (int i = 0; i < MAX_NODES; ++i)
        ntable[i].is_alive = false;
    return init_avl(&tree_ptr) && (mm_init_control_node() == mmr_ok);
}

bool deinit_control_node() {
    return deinit_avl(tree_ptr) && (mm_deinit_control_node() == mmr_ok);
}

execute_status execute_create(create_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    bool found = false;
    for (int i = 0; i < ntable_size; ++i) {
        if (ntable[i].info.id == cmd_info->id) {
            printf("Error: Already exists\n");
            status = es_bad_params;
            found = true;
            break;
        }
    }
    if (!found) {

        if (!add_to_tree(tree_ptr, cmd_info->id)) {
            LOG(LL_FATAL, "bad tree insert!");
            exit(EXIT_FAILURE);
        }
        ntable[ntable_size].is_alive = true;
        ntable[ntable_size].info.id = cmd_info->id;
        ntable[ntable_size].info.p_id = get_parent_id(tree_ptr, cmd_info->id);
        ntable_size++;

        __pid_t fork_res = fork();
        if (fork_res > 0) {
            LOG(LL_NOTE, "In Parent, created child with id: %d", cmd_info->id);

            ntable[ntable_size - 1].info.pid = fork_res;
            ntable[ntable_size - 1].info.ppid = getpid();

            if (mm_send_create(ntable[ntable_size - 1].info.id, ntable[ntable_size - 1].info.p_id) != mmr_ok) {
                status = es_msgq_error;
                printf("Error, couldn\'t link with node!\n");
            }
            else {
                printf("Ok: %d\n", fork_res);
            }
        }
        else if (fork_res == 0) {
            if (node_start(ntable[ntable_size - 1].info.id, ntable[ntable_size - 1].info.p_id) != ns_ok)
                exit(EXIT_FAILURE);
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
    LOG(LL_NOTE, "Killing child with id: %d", cmd_info->id);
    bool found = false;
    int index = -1;
    for (int i = 0; i < ntable_size; ++i) {
        if (ntable[i].info.id == cmd_info->id) {
            found = true;
            index = i;
            break;
        }
    }
    if (!found) {
        printf("Error: Not found");
        LOG(LL_NOTE, "no child with this id");
        status = es_bad_params;
    }
    else {
        if (mm_send_remove(ntable[index].info.id, ntable[index].info.p_id) == mmr_ok) {
            printf("Ok\n");
        }
        else {
            status = es_msgq_error;
            printf("Error: Node is unavailable\n");
        }
        ntable[index].is_alive = false;
    }
    return status;
}

execute_status execute_exec(exec_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    bool found = false;
    int index = -1;
    for (int i = 0; i < ntable_size; ++i) {
        if (ntable[i].info.id == cmd_info->id) {
            found = true;
            index = i;
        }
    }
    if (found) {
        mm_command cmd;
        cmd.pattern_len = cmd_info->pattern_len;
        cmd.text_len = cmd_info->text_len;
        strncpy(cmd.pattern, cmd_info->pattern, 256);
        strncpy(cmd.text, cmd_info->text, 256);
        if (mm_send_execute(&cmd, ntable[index].info.id, ntable[index].info.p_id) != mmr_ok)
            status = es_msgq_error;
    }
    else {
        status = es_bad_params;
    }
    return status;
}

execute_status execute_pingall(pingall_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    int root_id = get_root_id(tree_ptr);
    if (root_id != -1) {
        if (mm_send_pingall(get_root_id(tree_ptr)) != mmr_ok)
            status = es_msgq_error;
    }
    else {
        LOG(LL_NOTE, "Nothing to ping...");
    }
    // for (int i = 0; i < ntable_size; ++i) {
    //     if (ntable[i].is_alive) {
    //         int path_len;
    //         int* path = get_path(tree_ptr, ntable[i].info.id, &path_len);
    //         if (mm_send_ping(path, path_len) != mmr_ok) {
    //             status = es_msgq_error;
    //         }
    //     }
    // }
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
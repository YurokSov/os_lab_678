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
    if (ntable[cmd_info->id].is_alive == true) {
        printf("Error: Already exists\n");
        status = es_bad_params;
    }
    else {
        if (!add_to_tree(tree_ptr, cmd_info->id)) {
            LOG(LL_FATAL, "bad tree insert!");
            exit(EXIT_FAILURE);
        }
        ntable[cmd_info->id].is_alive = true;
        ntable[cmd_info->id].info.id = cmd_info->id;
        ntable[cmd_info->id].info.p_id = get_parent_id(tree_ptr, cmd_info->id);

        __pid_t fork_res = fork();
        if (fork_res > 0) {
            LOG(LL_NOTE, "In Parent, created child with id: %d", cmd_info->id);

            ntable[cmd_info->id].info.pid = fork_res;
            ntable[cmd_info->id].info.ppid = getpid();

            if (mm_send_create(ntable[cmd_info->id].info.id, ntable[cmd_info->id].info.p_id) != mmr_ok) {
                status = es_msgq_error;
                printf("Error, couldn\'t link with node!\n");
            }
            else {
                printf("Ok: %d\n", fork_res);
            }
        }
        else if (fork_res == 0) {
            if (node_start(ntable[cmd_info->id].info.id, ntable[cmd_info->id].info.p_id) != ns_ok)
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
    if (ntable[cmd_info->id].is_alive) {
        if (mm_send_remove(ntable[cmd_info->id].info.id, ntable[cmd_info->id].info.p_id) == mmr_ok) {
            printf("Ok\n");
            kill_child(ntable[cmd_info->id].info.pid);
            if (!remove_from_tree(tree_ptr, cmd_info->id))
                LOG(LL_ERROR, "REMOVE %d from tree ERROR!");
        }
        else {
            status = es_msgq_error;
            printf("Error: Node is unavailable\n");
        }
        ntable[cmd_info->id].is_alive = false;
    }
    else {
        printf("Error: Not found");
        LOG(LL_NOTE, "no child with this id");
        status = es_bad_params;
    }
    return status;
}

execute_status execute_exec(exec_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    bool found = false;
    if (ntable[cmd_info->id].is_alive) {
        mm_command cmd;
        cmd.pattern_len = cmd_info->pattern_len;
        cmd.text_len = cmd_info->text_len;
        strncpy(cmd.pattern, cmd_info->pattern, 256);
        strncpy(cmd.text, cmd_info->text, 256);
        cmd.pattern[cmd_info->pattern_len] = '\0';
        cmd.text[cmd_info->text_len] = '\0';
        LOG(LL_DEBUG, "in master: id:%d pattern:%s text:%s", ntable[cmd_info->id].info.id, cmd.pattern, cmd.text);
        if (mm_send_execute(&cmd, ntable[cmd_info->id].info.id) != mmr_ok) {
            status = es_msgq_error;
            printf("Error: node unavailable!\n");
        }
    }
    else {
        status = es_bad_params;
        printf("Error: no such node!\n");
    }
    return status;
}

execute_status execute_pingall(pingall_cmd* cmd_info, void** result) {
    execute_status status = es_ok;
    int root_id = get_root_pid(tree_ptr);
    if (root_id != -1) {
        int* alive = malloc(sizeof(int) * MAX_NODES);
        int len = 0;
        if (mm_send_pingall(root_id, alive, &len) != mmr_ok) {
            status = es_msgq_error;
        }
        printf("Ok:");

        bool is_used[MAX_NODES];
        for (int i = 0; i < MAX_NODES; ++i) {
            if (ntable[i].is_alive)
                is_used[i] = true;
            else
                is_used[i] = false;
            ntable[i].is_alive = false;
        }
        for (int i = 0; i < len; ++i) {
            ntable[alive[i]].is_alive = true;
        }
        bool flag = true;
        for (int i = 0; i < MAX_NODES; ++i) {
            if (is_used[i] && !ntable[i].is_alive) {
                printf("%d;", i);
                kill_child(ntable[i].info.pid);
                if (!remove_from_tree(tree_ptr, ntable[i].info.id))
                    LOG(LL_ERROR, "REMOVE %d from tree ERROR!");
                flag = false;
            }
        }
        if (flag) {
            printf("-1");
        }
        printf("\n");
    }
    else {
        LOG(LL_NOTE, "Nothing to ping...");
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

void kill_child(int pid) {
    if (kill(pid, SIGTERM) == ERROR_CODE) {
        LOG(LL_ERROR, "Error while killing child node!");
        perror("ERROR");
    }
    if (waitpid(pid, NULL, 0) == ERROR_CODE) {
        LOG(LL_ERROR, "Error while waiting child node!");
        perror("ERROR");
    }
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
#ifndef MM_INCLUDE_COMPUTING_NODE_H
#define MM_INCLUDE_COMPUTING_NODE_H

#include "../core/message_manager.h"

typedef enum mm_ecmd {
    mmc_create, mmc_remove, mmc_execurte, mmc_pingall,
} mm_ecmd;

typedef struct mm_cmd {
    mm_ecmd cmd;
    int length;
    void* buffer;
} mm_cmd;

mm_code mm_init_computing_node(int id, int p_id);

mm_code mm_deinit_computing_node(int id, int p_id);

mm_code mm_pass_create(int id, int p_id);

mm_code mm_pass_remove(int id, int p_id);

mm_code mm_pass_execute(mm_command cmd, int id, int p_id);

mm_code mm_pass_pingall();

mm_cmd mm_recv_command();

#endif
#ifndef MM_INCLUDE_COMPUTING_NODE_H
#define MM_INCLUDE_COMPUTING_NODE_H

#include "../core/message_manager.h"

typedef int ping_cmd;

mm_code mm_init_computing_node(int id, int p_id);

mm_code mm_deinit_computing_node();

mm_code mm_pass_rebind(int id, mm_command* msg);

mm_code mm_pass_relax();

//mm_code mm_pass_create(int this_id, int this_p_id, int id, int p_id);

//mm_code mm_pass_remove(int this_id, int this_p_id, int id, int p_id);

mm_code mm_pass_execute(int id, mm_command* msg);

mm_code mm_pass_pingall();

void mm_recv_command();

#endif
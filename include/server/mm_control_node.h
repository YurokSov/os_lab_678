#ifndef INCLUDE_MM_CONTROL_NODE_H
#define INCLUDE_MM_CONTROL_NODE_H

#include "../core/message_manager.h"

mm_code mm_init_control_node();

mm_code mm_deinit_control_node();

mm_code mm_send_rebind(int id, int p_id);

mm_code mm_send_relax(int id, int p_id);

mm_code mm_send_create(int id, int p_id);

mm_code mm_send_remove(int id, int p_id);

mm_code mm_send_execute(mm_command* cmd, int id, int p_id);

mm_code mm_send_pingall(int root_id, int* alive, int* len);

#endif
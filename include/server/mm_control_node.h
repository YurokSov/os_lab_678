#ifndef INCLUDE_MM_CONTROL_NODE_H
#define INCLUDE_MM_CONTROL_NODE_H

#include "../core/message_manager.h"



mm_code mm_init_control_node();

mm_code mm_deinit_control_node();

mm_code mm_send_rebind(int p_id, int id);

mm_code mm_send_relax(int p_id, int id);

mm_code mm_send_create(int* path, int path_len);

mm_code mm_send_remove(int* path, int path_len);

mm_code mm_send_exec(search_pat_in_text cmd, int* path, int path_len);

mm_code mm_send_ping(int* path, int path_len);

#endif
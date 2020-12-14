#include "server/mm_control_node.h"

#include "utils/logger.h"
#include "zmq.h"
//#include "zmq_utils.h"

static zctx_t context;
static zsock_t ping_sub;
static zsock_t root_pub;

static int recv_timeout_ms = 100;
static int node_connect_timeout_ms = 100;

mm_code mm_init_control_node() {
    context = zmq_ctx_new();
    CTX_CREAT_ERR_CHK(context);
    ping_sub = zmq_socket(context, ZMQ_SUB);
    SOCK_CREAT_ERR_CHK(ping_sub);

    SETSOCKOPT_ERR_CHK(ping_sub, zmq_setsockopt(ping_sub, ZMQ_SUBSCRIBE, NULL, 0));
    SETSOCKOPT_ERR_CHK(ping_sub, zmq_setsockopt(ping_sub, ZMQ_RCVTIMEO, &recv_timeout_ms, sizeof recv_timeout_ms));
    SETSOCKOPT_ERR_CHK(ping_sub, zmq_setsockopt(ping_sub, ZMQ_CONNECT_TIMEOUT, &node_connect_timeout_ms, sizeof node_connect_timeout_ms));
    SOCK_BIND_ERR_CHK(ping_sub, zmq_bind(ping_sub, MASTER_PING));
    root_pub = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(root_pub);
    SOCK_BIND_ERR_CHK(root_pub, zmq_bind(root_pub, MASTER_ROOT));
    return mmr_ok;
}

mm_code mm_deinit_control_node() {
    SOCK_UNBIND_ERR_CHK(ping_sub, zmq_unbind(ping_sub, MASTER_PING));
    SOCK_UNBIND_ERR_CHK(root_pub, zmq_unbind(ping_sub, MASTER_ROOT));
    SOCK_CLOSE_ERR_CHK(ping_sub, zmq_close(ping_sub));
    SOCK_CLOSE_ERR_CHK(root_pub, zmq_close(root_pub));
    CTX_TERM_ERR_CHK(zmq_ctx_term(context));
    return mmr_ok;
}

// mm_code mm_subscribe_control_node(int id) {
//     mm_code status = mmr_ok;
//     char buffer[SOCK_BUF_LEN] = SOCK_PREFIX;
//     if (id < 0 || snprintf(buffer + sizeof SOCK_PREFIX, SOCK_BUF_LEN, "%d", id)) {
//         LOG(LL_ERROR, "Bad ID provided!");
//     }
//     zmq_connect(ping_sub, buffer);
//     return status;
// }

// mm_code mm_unsubscribe_control_node(int id) {
//     mm_code status = mmr_ok;

//     return status;
// }


mm_code mm_send_rebind(int id, int p_id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_relax(int id, int p_id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_create(int id, int p_id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_remove(int id, int p_id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_execute(mm_command cmd, int id, int p_id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_pingall(int root_id) {
    //TODO
    LOG(LL_NOTE, "Sending pingall to root node with id = %d.", root_id);
    return mmr_ok;
}
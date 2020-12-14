#include "server/mm_control_node.h"

#include "utils/logger.h"
#include "zmq.h"
#include "zmq_utils.h"
#include "string.h"

static zctx_t context;
static zsock_t ping_sub;
static zsock_t root_pub;

static int recv_timeout_ms = 100;
static int node_connect_timeout_ms = 100;

#define SOCK_PREFIX "ipc://lab/"

mm_code mm_init_control_node() {
    mm_code status = mmr_ok;

    context = zmq_ctx_new();
    if (context == NULL) {
        LOG(LL_FATAL, "couldn\'t create zmq_context");
        perror("zmq_ctx_new ");
        return mmr_error;
    }

    ping_sub = zmq_socket(context, ZMQ_SUB);
    if (ping_sub == NULL) {
        LOG(LL_FATAL, "couldn\'t create ping_pub socket");
        perror("zmq_socket ");
        return mmr_error;
    }
    zmq_setsockopt(ping_sub, ZMQ_SUBSCRIBE, NULL, 0);
    zmq_setsockopt(ping_sub, ZMQ_RCVTIMEO, &recv_timeout_ms, sizeof recv_timeout_ms);
    zmq_setsockopt(ping_sub, ZMQ_CONNECT_TIMEOUT, &node_connect_timeout_ms, sizeof node_connect_timeout_ms);

    if (zmq_bind(ping_sub, MASTER_PING)) {
        LOG(LL_FATAL, "ping_sub bind error!");
        perror("zmq_bind ");
        return mmr_error;
    }

    root_pub = zmq_socket(context, ZMQ_PUB);
    if (ping_sub == NULL) {
        LOG(LL_FATAL, "couldn\'t create ping_pub socket");
        perror("zmq_socket ");
        return mmr_error;
    }
    if (zmq_bind(root_pub, MASTER_ROOT)) {
        LOG(LL_FATAL, "root_ping bind error!");
        perror("zmq_bind ");
        return mmr_error;
    }

    return status;
}

mm_code mm_deinit_control_node() {
    mm_code status = mmr_ok;

    if (zmq_unbind(ping_sub, MASTER_PING)) {
        LOG(LL_FATAL, "ping_sub unbind error!");
        perror("zmq_unbind ");
        return mmr_error;
    }

    if (zmq_unbind(root_pub, MASTER_ROOT)) {
        LOG(LL_FATAL, "root_pub unbind error!");
        perror("zmq_unbind ");
        return mmr_error;
    }

    if (zmq_close(ping_sub)) {
        LOG(LL_FATAL, "ping_sub close error!");
        perror("zmq_close ");
        return mmr_error;
    }

    if (zmq_close(root_pub)) {
        LOG(LL_FATAL, "root_pub close error!");
        perror("zmq_close ");
        return mmr_error;
    }

    if (zmq_ctx_term(context)) {
        LOG(LL_FATAL, "context terminate error!");
        perror("zmq_ctx_term ");
        return mmr_error;
    }

    return status;
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


mm_code mm_send_rebind(int p_id, int id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_relax(int p_id, int id) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_create(int* path, int path_len) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_remove(int* path, int path_len) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_exec(search_pat_in_text cmd, int* path, int path_len) {
    //TODO
    return mmr_ok;
}

mm_code mm_send_ping(int* path, int path_len) {
    //TODO
    return mmr_ok;
}
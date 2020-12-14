#include "client/mm_computing_node.h"

#include "utils/logger.h"
#include "zmq.h"
//#include "zmq_utils.h"

static zctx_t context;
static zsock_t psock_s;
static zsock_t lsock_p;
static zsock_t rsock_p;
static zsock_t ping_sock;

static char node_endpoint[SOCK_BUF_LEN] = SLAVE_PREFIX;
static char parent_endpoint[SOCK_BUF_LEN] = SLAVE_PREFIX;
static int recv_timeout_ms = 100;
static int node_connect_timeout_ms = 100;

mm_code mm_init_computing_node(int id, int p_id) {
    context = zmq_ctx_new();
    if (context == NULL) {
        LOG(LL_FATAL, "couldn\'t create zmq_context");
        perror("zmq_ctx_new ");
        return mmr_error;
    }

    psock_s = zmq_socket(context, ZMQ_SUB);
    SOCK_CREAT_ERR_CHK(psock_s);
    SETSOCKOPT_ERR_CHK(psock_s, zmq_setsockopt(psock_s, ZMQ_SUBSCRIBE, NULL, 0));
    SETSOCKOPT_ERR_CHK(psock_s, zmq_setsockopt(psock_s, ZMQ_CONNECT_TIMEOUT, &node_connect_timeout_ms, sizeof node_connect_timeout_ms));

    if (snprintf(parent_endpoint + sizeof SLAVE_PREFIX, SOCK_BUF_LEN, "%d", p_id) == 0) {
        LOG(LL_ERROR, "Bad parent id!");
        return mmr_bad_params;
    }
    SOCK_BIND_ERR_CHK(psock_s, zmq_bind(psock_s, parent_endpoint));

    lsock_p = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(lsock_p);
    rsock_p = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(rsock_p);
    ping_sock = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(ping_sock);
    SOCK_CONNECT_ERR_CHK(ping_sock, zmq_connect(ping_sock, MASTER_PING));

    return mmr_ok;
}

static char this_ep[] = "ipc://lab/node/5";
static char left_ep[] = "ipc://lab/node/1";
static char right_ep[] = "ipc://lab/node/9";

mm_code mm_deinit_computing_node(int id, int p_id) {
    SOCK_DISCONNECT_ERR_CHK(ping_sock, zmq_disconnect(ping_sock, MASTER_PING));
    SOCK_CLOSE_ERR_CHK(ping_sock, zmq_close(ping_sock));
    SOCK_CLOSE_ERR_CHK(rsock_p, zmq_close(rsock_p));
    SOCK_CLOSE_ERR_CHK(lsock_p, zmq_close(lsock_p));
    SOCK_UNBIND_ERR_CHK(psock_s, zmq_unbind(psock_s, parent_endpoint));
    SOCK_CLOSE_ERR_CHK(psock_s, zmq_close(psock_s));
    CTX_TERM_ERR_CHK(zmq_ctx_term(context));
}

mm_code mm_pass_create(int id, int p_id) {
    // if (p_id == this_id) {

    // }
    // else {

    // }
}

mm_code mm_pass_remove(int id, int p_id) {

}

mm_code mm_pass_execute(mm_command cmd, int id, int p_id) {

}

mm_code mm_pass_pingall() {

}

mm_cmd mm_recv_command() {

}
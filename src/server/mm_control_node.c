#include "server/mm_control_node.h"

#include "utils/logger.h"
#include "zmq.h"
//#include "zmq_utils.h"

#include <stdlib.h>
#include <string.h>

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
    LOG(LL_DEBUG, "root_ping sub bound ON %s", MASTER_PING);
    root_pub = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(root_pub);
    SOCK_BIND_ERR_CHK(root_pub, zmq_bind(root_pub, MASTER_ROOT));
    LOG(LL_DEBUG, "root_pub bound on %s", MASTER_ROOT);
    return mmr_ok;
}

mm_code mm_deinit_control_node() {
    SOCK_UNBIND_ERR_CHK(ping_sub, zmq_unbind(ping_sub, MASTER_PING));
    SOCK_UNBIND_ERR_CHK(root_pub, zmq_unbind(root_pub, MASTER_ROOT));
    SOCK_CLOSE_ERR_CHK(ping_sub, zmq_close(ping_sub));
    SOCK_CLOSE_ERR_CHK(root_pub, zmq_close(root_pub));
    do {
        if (!zmq_ctx_term(context))
            break;
        LOG(LL_FATAL, "couldn\'t terminate zmq_context! errno: %d", errno);
        perror("zmq_ctx_term ");
        sleep(5);
    } while (errno == EAGAIN);
    return mmr_ok;
}

mm_code mm_send_rebind(int id, int target_id) {
    LOG(LL_NOTE, "SEND REBIND TO {%d;%d}", id, target_id);
    mm_cmd sent_cmd;
    sent_cmd.id = id;
    sent_cmd.cmd = mmc_rebind;
    sent_cmd.length = 4;
    sent_cmd.buffer.temp_id = target_id;
    zmq_msg_t zmqmsg;
    zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
    memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));
    mm_cmd* test = zmq_msg_data(&zmqmsg);
    int send = zmq_msg_send(&zmqmsg, root_pub, 0);
    zmq_msg_close(&zmqmsg);
    return mmr_ok;
}

mm_code mm_send_relax() {
    LOG(LL_NOTE, "SEND RELAX TO ALL");
    mm_cmd sent_cmd;
    sent_cmd.cmd = mmc_relax;
    sent_cmd.length = 0;
    zmq_msg_t zmqmsg;
    zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
    memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));
    mm_cmd* test = zmq_msg_data(&zmqmsg);
    int send = zmq_msg_send(&zmqmsg, root_pub, 0);
    zmq_msg_close(&zmqmsg);
    return mmr_ok;
}

mm_code mm_send_execute(mm_command* cmd, int id) {
    mm_cmd sent_cmd;
    sent_cmd.id = id;
    sent_cmd.cmd = mmc_execute;
    sent_cmd.length = sizeof(*cmd);
    sent_cmd.buffer.pattern_len = cmd->pattern_len;
    sent_cmd.buffer.text_len = cmd->text_len;
    memcpy(sent_cmd.buffer.pattern, cmd->pattern, CMD_MAX_BUF_SIZE);
    memcpy(sent_cmd.buffer.text, cmd->text, CMD_MAX_BUF_SIZE);

    zmq_msg_t zmqmsg;
    zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
    memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));

    mm_cmd* test = zmq_msg_data(&zmqmsg);
    // LOG(LL_DEBUG, "TEST: id:%d len:%d, cmd:%d, {%d,%d,%s,%s}",
    //     test->id, test->length, test->cmd, test->buffer.pattern_len,
    //     test->buffer.text_len, test->buffer.pattern, test->buffer.text);

    int send = zmq_msg_send(&zmqmsg, root_pub, 0);
    zmq_msg_close(&zmqmsg);

    return mmr_ok;
}

mm_code mm_send_pingall(int root_id, int* alive, int* len) {
    LOG(LL_NOTE, "Sending pingall to root node with id = %d.", root_id);
    {
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_ecmd));
        mm_ecmd* data = zmq_msg_data(&zmqmsg);
        *data = mmc_pingall;
        zmq_msg_send(&zmqmsg, root_pub, 0);
        zmq_msg_close(&zmqmsg);
    }
    //sleep(1);
    int p = 0;
    do {
        zmq_msg_t zmqmsg;
        zmq_msg_init(&zmqmsg);
        if (zmq_msg_recv(&zmqmsg, ping_sub, 0) == -1) {
            if (errno == EAGAIN) {
                LOG(LL_DEBUG, "NO MORE ALIVE NODES!!");
                zmq_msg_close(&zmqmsg);
                break;
            }
        }
        int id = *((int*)zmq_msg_data(&zmqmsg));
        alive[p] = id;
        p++;
        LOG(LL_NOTE, "NODE WITH ID %d IS ALIVE!", id);
        zmq_msg_close(&zmqmsg);
    } while (true);
    *len = p;
    return mmr_ok;
}
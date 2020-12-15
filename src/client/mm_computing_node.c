#include "client/mm_computing_node.h"
#include "client/computing_node.h"

#include "utils/logger.h"
#include "zmq.h"
//#include "zmq_utils.h"

#include <stdlib.h>
#include <string.h>

static zctx_t context;
static zsock_t psock_s;
static zsock_t lsock_p;
static zsock_t rsock_p;
static zsock_t ping_sock;

static int temp_id;
static int this_id;
static int this_p_id;

static char parent_endpoint[SOCK_BUF_LEN] = SLAVE_PREFIX;
static char left_endpoint[SOCK_BUF_LEN] = SLAVE_PREFIX;
static char right_endpoint[SOCK_BUF_LEN] = SLAVE_PREFIX;

static int recv_timeout_ms = 100;
static int node_connect_timeout_ms = 100;

mm_cmd* message;

mm_code mm_init_computing_node(int id, int p_id) {
    //LOG(LL_DEBUG, "inside init node, p_id = %d, id = %d", p_id, id);
    this_p_id = p_id;
    this_id = id;
    temp_id = this_p_id;
    context = zmq_ctx_new();
    CTX_CREAT_ERR_CHK(context);
    psock_s = zmq_socket(context, ZMQ_SUB);
    SOCK_CREAT_ERR_CHK(psock_s);
    SETSOCKOPT_ERR_CHK(psock_s, zmq_setsockopt(psock_s, ZMQ_SUBSCRIBE, NULL, 0));
    SETSOCKOPT_ERR_CHK(psock_s, zmq_setsockopt(psock_s, ZMQ_CONNECT_TIMEOUT, &node_connect_timeout_ms, sizeof node_connect_timeout_ms));

    if (id > p_id) {
        if (sprintf(parent_endpoint + sizeof SLAVE_PREFIX - 1, "%d-right", p_id) == 0) {
            LOG(LL_ERROR, "Bad parent id!");
            return mmr_bad_params;
        }
    }
    else {
        if (sprintf(parent_endpoint + sizeof SLAVE_PREFIX - 1, "%d-left", p_id) == 0) {
            LOG(LL_ERROR, "Bad parent id!");
            return mmr_bad_params;
        }
    }
    SOCK_CONNECT_ERR_CHK(psock_s, zmq_connect(psock_s, parent_endpoint));
    //LOG(LL_DEBUG, "psock_s of %d is connected to parent at %s", id, parent_endpoint);

    lsock_p = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(lsock_p);
    sprintf(left_endpoint + sizeof SLAVE_PREFIX - 1, "%d-left", id);
    SOCK_BIND_ERR_CHK(lsock_p, zmq_bind(lsock_p, left_endpoint));
    //LOG(LL_DEBUG, "lsock_p of %d is bound to left at %s", id, left_endpoint);

    rsock_p = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(rsock_p);
    sprintf(right_endpoint + sizeof SLAVE_PREFIX - 1, "%d-right", id);
    SOCK_BIND_ERR_CHK(rsock_p, zmq_bind(rsock_p, right_endpoint));
    //LOG(LL_DEBUG, "rsock_p of %d is bound to right at %s", id, right_endpoint);

    ping_sock = zmq_socket(context, ZMQ_PUB);
    SOCK_CREAT_ERR_CHK(ping_sock);
    SOCK_CONNECT_ERR_CHK(ping_sock, zmq_connect(ping_sock, MASTER_PING));
    //LOG(LL_DEBUG, "ID %d CONNECTED TO PING_SUB ON %s", id, MASTER_PING);
    return mmr_ok;
}

mm_code mm_deinit_computing_node() {
    SOCK_DISCONNECT_ERR_CHK(ping_sock, zmq_disconnect(ping_sock, MASTER_PING));
    SOCK_CLOSE_ERR_CHK(ping_sock, zmq_close(ping_sock));
    SOCK_UNBIND_ERR_CHK(rsock_p, zmq_unbind(rsock_p, right_endpoint));
    SOCK_CLOSE_ERR_CHK(rsock_p, zmq_close(rsock_p));
    SOCK_UNBIND_ERR_CHK(lsock_p, zmq_unbind(lsock_p, left_endpoint));
    SOCK_CLOSE_ERR_CHK(lsock_p, zmq_close(lsock_p));
    SOCK_UNBIND_ERR_CHK(psock_s, zmq_unbind(psock_s, parent_endpoint));
    SOCK_CLOSE_ERR_CHK(psock_s, zmq_close(psock_s));
    do {
        if (!zmq_ctx_term(context))
            break;
        LOG(LL_FATAL, "couldn\'t terminate zmq_context! errno: %d", errno);
        perror("zmq_ctx_term ");
        sleep(5);
    } while (errno == EAGAIN);
}

mm_code mm_pass_rebind(int id, mm_command* cmd) {
    if (this_id == id) {
        //LOG(LL_DEBUG, "Change TEMP_ID IN NODE %d from %d to %d", this_id, temp_id, cmd->temp_id);
        temp_id = cmd->temp_id;
    }
    else {
        mm_cmd sent_cmd;
        sent_cmd.cmd = mmc_rebind;
        sent_cmd.id = id;
        sent_cmd.length = sizeof(mm_command);
        sent_cmd.buffer.temp_id = cmd->temp_id;
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
        memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));
        if (id < this_id) {
            //LOG(LL_DEBUG, "sent rebind(%d) from %d to left", id, this_id);
            zmq_msg_send(&zmqmsg, lsock_p, 0);
        }
        else {
            //LOG(LL_DEBUG, "sent rebind(%d) from %d to right", id, this_id);
            zmq_msg_send(&zmqmsg, rsock_p, 0);
        }
        zmq_msg_close(&zmqmsg);
    }
}

mm_code mm_pass_relax() {
    {
        mm_cmd sent_cmd;
        sent_cmd.cmd = mmc_relax;
        sent_cmd.length = sizeof(mm_command);
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
        memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));
        //LOG(LL_DEBUG, "SENDING L relax from %d", this_id);
        zmq_msg_send(&zmqmsg, lsock_p, 0);
        zmq_msg_close(&zmqmsg);
    }
    {
        mm_cmd sent_cmd;
        sent_cmd.cmd = mmc_relax;
        sent_cmd.length = sizeof(mm_command);
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
        memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));
        //LOG(LL_DEBUG, "SENDING R relax from %d", this_id);
        zmq_msg_send(&zmqmsg, rsock_p, 0);
        zmq_msg_close(&zmqmsg);
    }
    {
        //
        if (this_p_id != temp_id) {
            //LOG(LL_DEBUG, "RELAXING>........");
            //LOG(LL_DEBUG, "RELAX TEMP_ID IN NODE %d FROM %d TO %d", this_id, this_p_id, temp_id);
            SOCK_DISCONNECT_ERR_CHK(psock_s, zmq_disconnect(psock_s, parent_endpoint));
            if (this_id < temp_id)
                sprintf(parent_endpoint + sizeof SLAVE_PREFIX - 1, "%d-left", temp_id);
            else
                sprintf(parent_endpoint + sizeof SLAVE_PREFIX - 1, "%d-right", temp_id);
            SOCK_CONNECT_ERR_CHK(psock_s, zmq_connect(psock_s, parent_endpoint));
            //LOG(LL_DEBUG, "Node %d psock_s connected to %s", this_id, parent_endpoint);
            this_p_id = temp_id;
        }
        else {
            //LOG(LL_DEBUG, "NO RELAXING NEEDED!!!");
        }
    }
}

mm_code mm_pass_execute(int id, mm_command* msg) {
    //LOG(LL_DEBUG, "on node %d execute recieved id %d and {%s,%s}",
    //    this_id, id, msg->pattern, msg->text);
    if (this_id == id) {
        execute_command(msg);
        return mmr_ok;
    }
    else {
        mm_cmd sent_cmd;
        sent_cmd.cmd = mmc_execute;
        sent_cmd.id = id;
        sent_cmd.length = sizeof(mm_command);
        sent_cmd.buffer.pattern_len = msg->pattern_len;
        sent_cmd.buffer.text_len = msg->text_len;
        memcpy(sent_cmd.buffer.pattern, msg->pattern, CMD_MAX_BUF_SIZE);
        memcpy(sent_cmd.buffer.text, msg->text, CMD_MAX_BUF_SIZE);
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
        memcpy(zmq_msg_data(&zmqmsg), &sent_cmd, sizeof(mm_cmd));
        if (id < this_id) {
            //LOG(LL_DEBUG, "sent exec(%d) from %d to left", id, this_id);
            zmq_msg_send(&zmqmsg, lsock_p, 0);
        }
        else {
            //LOG(LL_DEBUG, "sent exec(%d) from %d to right", id, this_id);
            zmq_msg_send(&zmqmsg, rsock_p, 0);
        }
        zmq_msg_close(&zmqmsg);
    }
}

mm_code mm_pass_pingall() {
    {
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_ecmd));
        mm_ecmd* data = zmq_msg_data(&zmqmsg);
        *data = mmc_pingall;
        //LOG(LL_DEBUG, "NODE: %d SENDING L pingall", this_id);
        zmq_msg_send(&zmqmsg, lsock_p, 0);
        zmq_msg_close(&zmqmsg);
    }
    {
        zmq_msg_t zmqmsg;
        zmq_msg_init_size(&zmqmsg, sizeof(mm_ecmd));
        mm_ecmd* data = zmq_msg_data(&zmqmsg);
        *data = mmc_pingall;
        //LOG(LL_DEBUG, "NODE: %d SENDING R pingall", this_id);
        zmq_msg_send(&zmqmsg, rsock_p, 0);
        zmq_msg_close(&zmqmsg);
    }
    //LOG(LL_DEBUG, "SENDING pingall done in %d!", this_id);
    {
        zmq_msg_t pingmsg;
        zmq_msg_init_size(&pingmsg, sizeof(int));
        int* data = zmq_msg_data(&pingmsg);
        *data = this_id;
        zmq_msg_send(&pingmsg, ping_sock, 0);
        LOG(LL_DEBUG, "ID %d SENT PING MSG TO PARENT!", this_id);
        zmq_msg_close(&pingmsg);
    }
}

void mm_recv_command() {
    zmq_msg_t zmqmsg;
    zmq_msg_init_size(&zmqmsg, sizeof(mm_cmd));
    zmq_msg_recv(&zmqmsg, psock_s, 0);
    mm_cmd* test = zmq_msg_data(&zmqmsg);
    message = (mm_cmd*)zmq_msg_data(&zmqmsg);
    mm_command buffer;
    if (message->cmd == mmc_execute) {
        buffer.pattern_len = message->buffer.pattern_len;
        buffer.text_len = message->buffer.text_len;
        memcpy(buffer.pattern, message->buffer.pattern, CMD_MAX_BUF_SIZE);
        memcpy(buffer.text, message->buffer.text, CMD_MAX_BUF_SIZE);
    }
    else if (message->cmd == mmc_rebind) {
        buffer.temp_id = message->buffer.temp_id;
    }
    int id = message->id;
    int cmd = message->cmd;

    zmq_msg_close(&zmqmsg);
    //LOG(LL_DEBUG, "TEST %d {%d,%d,%s,%s}", id, buffer.pattern_len, buffer.text_len, buffer.pattern, buffer.text);
    switch (cmd) {
    case mmc_rebind:
        LOG(LL_DEBUG, "Node %d Recieved REBIND!", this_id);
        mm_pass_rebind(id, &buffer);
        break;
    case mmc_relax:
        LOG(LL_DEBUG, "Node %d Recieved RELAX!", this_id);
        mm_pass_relax();
        break;
    case mmc_execute:
        LOG(LL_DEBUG, "Node %d Recieved EXEC!", this_id);
        mm_pass_execute(id, &buffer);
        break;
    case mmc_pingall:
        LOG(LL_DEBUG, "Node %d Recieved PINGALL!", this_id);
        mm_pass_pingall();
        break;
    default:
        LOG(LL_WARNING, "Node %d Recieved UNSUPPORTED MESSAGE: %d", this_id, cmd);
    }
}
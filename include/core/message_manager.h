#ifndef INCLUDE_MESSAGE_MANAGER_H
#define INCLUDE_MESSAGE_MANAGER_H

#include "defines.h"

typedef enum mm_code {
    mmr_ok, mmr_error, mmr_bad_params,
} mm_code;

#define CMD_MAX_BUF_SIZE 256

typedef struct mm_command {
    int pattern_len;
    int text_len;
    char pattern[CMD_MAX_BUF_SIZE];
    char text[CMD_MAX_BUF_SIZE];
} mm_command;

typedef enum mm_ecmd {
    mmc_create, mmc_remove, mmc_execute, mmc_pingall,
} mm_ecmd;

typedef struct mm_cmd {
    mm_ecmd cmd;
    int length;
    int id;
    int p_id;
    mm_command buffer;
    //char buffer[sizeof(mm_command)];
} mm_cmd;

typedef void* zctx_t;
typedef void* zsock_t;

#define MASTER_ROOT "ipc://@lab/-1-right"

#define MASTER_PING "ipc://@lab/ping"

#define SLAVE_PREFIX "ipc://@lab/"

#define SOCK_BUF_LEN 113

#define CTX_CREAT_ERR_CHK(context) \
    if (context == NULL) { \
        LOG(LL_FATAL, "couldn\'t create zmq_context! errno: %d", errno); \
        perror("zmq_ctx_new "); \
    } \

#define CTX_TERM_ERR_CHK(result) \
    if (context) { \
        LOG(LL_FATAL, "couldn\'t terminate zmq_context! errno: %d", errno); \
        perror("zmq_ctx_term "); \
    } \

#define SOCK_CREAT_ERR_CHK(socket) \
    if (socket == NULL) { \
        LOG(LL_FATAL, "couldn\'t create " xstr(socket) " socket! errno: %d", errno); \
        perror("zmq_socket "); \
    } \

#define SOCK_CLOSE_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, "couldn\'t close " xstr(socket) " socket! errno: %d", errno); \
        perror("zmq_close "); \
    } \

#define SETSOCKOPT_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, "couldn\'t set " xstr(socket) " options! errno: %d", errno); \
        perror("zmq_setsockopt "); \
    } \

#define SOCK_BIND_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " bind error! errno: %d", errno); \
        perror("zmq_bind "); \
    } \

#define SOCK_UNBIND_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " unbind error! errno: %d", errno); \
        perror("zmq_unbind "); \
    } \

#define SOCK_CONNECT_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " connect error! errno: %d", errno); \
        perror("zmq_connect "); \
    } \

#define SOCK_DISCONNECT_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " disconnect error! errno: %d", errno); \
        perror("zmq_disconnect "); \
    } \

#endif
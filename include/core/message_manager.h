#ifndef INCLUDE_MESSAGE_MANAGER_H
#define INCLUDE_MESSAGE_MANAGER_H

#include "defines.h"

typedef enum mm_code {
    mmr_ok, mmr_error, mmr_bad_params,
} mm_code;

typedef struct mm_command {
    int pattern_len;
    int text_len;
    char* pattern;
    char* text;
} mm_command;

typedef void* zctx_t;
typedef void* zsock_t;

#define MASTER_ROOT "ipc://@lab/control-root"

#define MASTER_PING "ipc://@lab/control-ping"

#define SLAVE_PREFIX "ipc://@lab/"

#define SOCK_BUF_LEN 113

#define CTX_CREAT_ERR_CHK(context) \
    if (context == NULL) { \
        LOG(LL_FATAL, "couldn\'t create zmq_context! errno: %d", errno); \
        perror("zmq_ctx_new "); \
        return mmr_error; \
    } \

#define CTX_TERM_ERR_CHK(result) \
    if (context) { \
        LOG(LL_FATAL, "couldn\'t terminate zmq_context! errno: %d", errno); \
        perror("zmq_ctx_term "); \
        return mmr_error; \
    } \

#define SOCK_CREAT_ERR_CHK(socket) \
    if (socket == NULL) { \
        LOG(LL_FATAL, "couldn\'t create " xstr(socket) " socket! errno: %d", errno); \
        perror("zmq_socket "); \
        return mmr_error; \
    } \

#define SOCK_CLOSE_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, "couldn\'t close " xstr(socket) " socket! errno: %d", errno); \
        perror("zmq_close "); \
        return mmr_error; \
    } \

#define SETSOCKOPT_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, "couldn\'t set " xstr(socket) " options! errno: %d", errno); \
        perror("zmq_setsockopt "); \
        return mmr_error; \
    } \

#define SOCK_BIND_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " bind error! errno: %d", errno); \
        perror("zmq_bind "); \
        return mmr_error; \
    } \

#define SOCK_UNBIND_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " unbind error! errno: %d", errno); \
        perror("zmq_unbind "); \
        return mmr_error; \
    } \

#define SOCK_CONNECT_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " connect error! errno: %d", errno); \
        perror("zmq_connect "); \
        return mmr_error; \
    } \

#define SOCK_DISCONNECT_ERR_CHK(socket, result) \
    if (result) { \
        LOG(LL_FATAL, xstr(socket) " disconnect error! errno: %d", errno); \
        perror("zmq_disconnect "); \
        return mmr_error; \
    } \

#endif
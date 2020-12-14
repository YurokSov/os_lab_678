#ifndef INCLUDE_MESSAGE_MANAGER_H
#define INCLUDE_MESSAGE_MANAGER_H

typedef enum mm_code {
    mmr_ok, mmr_error,
} mm_code;

typedef struct search_pat_in_text {
    int pattern_len;
    int text_len;
    char* pattern;
    char* text;
} search_pat_in_text;

typedef void* zctx_t;

typedef void* zsock_t;

#define MASTER_ROOT "ipc://lab/node/control-root"

#define MASTER_PING "ipc://lab/node/control-ping"

#define SLAVE_PUB "ipc://lab/node/computer/"

#define SOCK_BUF_LEN 40

#endif
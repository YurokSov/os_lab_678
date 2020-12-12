#ifndef INCLUDE_MESSAGE_MANAGER_H
#define INCLUDE_MESSAGE_MANAGER_H

typedef enum msgq_result {
    mmr_ok, mmr_error,
} msgq_result;

typedef struct search_pat_in_text {
    int pattern_len;
    int text_len;
    char* pattern;
    char* text;
} search_pat_in_text;

msgq_result mm_send_creat(int* path, int path_len);

msgq_result mm_send_remov(int* path, int path_len);

msgq_result mm_send_exec(search_pat_in_text cmd, int* path, int path_len);

msgq_result mm_send_ping(int* path, int path_len);

#endif
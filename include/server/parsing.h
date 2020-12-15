#ifndef INCLUDE_PARSING_H
#define INCLUDE_PARSING_H

#include "../core/defines.h"

typedef enum cmd_enum {
    ce_create, ce_remove, ce_exec, ce_pingall, ce_print,
} cmd_enum;

typedef enum parse_status {
    pse_ok, pse_error, pse_eof,
} parse_status;

typedef struct create_cmd {
    DWORD id;
    // DWORD p_id;
} create_cmd;

typedef struct remove_cmd {
    DWORD id;
} remove_cmd;

typedef struct exec_cmd {
    DWORD id;
    DWORD text_len;
    DWORD pattern_len;
    char* text;
    char* pattern;
} exec_cmd;

typedef struct pingall_cmd {
} pingall_cmd;

typedef struct print_cmd {
} print_cmd;

typedef union command_u {
    create_cmd create;
    remove_cmd remove;
    exec_cmd exec;
    pingall_cmd pingall;
    print_cmd print;
} command_u;

parse_status parse_command(cmd_enum* cmd);

parse_status get_cmd_info(cmd_enum* cmd, command_u* cmd_info);

#endif
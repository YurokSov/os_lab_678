#include "parsing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "logger.h"

parse_status parse_command(cmd_enum* cmd) {
    parse_status status = pse_ok;
    char* command = malloc(STRBUF_LEN * sizeof(char));
    printf("> ");
    if (scanf("%s", command) == EOF)
        return pse_eof;
    if (!strcmp(command, "create"))
        *cmd = ce_create;
    else if (!strcmp(command, "remove"))
        *cmd = ce_remove;
    else if (!strcmp(command, "exec"))
        *cmd = ce_exec;
    else if (!strcmp(command, "pingall"))
        *cmd = ce_pingall;
    else
        status = pse_error;
    free(command);
    return status;
}

parse_status parse_create(create_cmd* cmd) {
    //if (scanf("%d %d", &cmd->id, &cmd->parent_id) != 2)
    //    return pse_error;
    if (scanf("%d", &cmd->id) != 1)
        return pse_error;
    return pse_ok;
}

parse_status parse_remove(remove_cmd* cmd) {
    if (scanf("%d", &cmd->id) != 1)
        return pse_error;
    return pse_ok;
}

parse_status parse_exec(exec_cmd* cmd) {
    if (scanf("%d", &cmd->id) != 1)
        return pse_error;

    cmd->text = NEW_STRING;
    if (cmd->text == NULL) {
        LOG(LL_FATAL, "BAD ALLOC");
        exit(BAD_ALLOC);
    }
    cmd->pattern = NEW_STRING;
    if (cmd->text == NULL) {
        LOG(LL_FATAL, "BAD ALLOC");
        exit(BAD_ALLOC);
    }

    printf("> ");
    if (scanf("%s", cmd->text) != 1)
        return pse_error;
    cmd->text_len = strlen(cmd->text);
    printf("> ");
    if (scanf("%s", cmd->pattern) != 1)
        return pse_error;
    cmd->pattern_len = strlen(cmd->pattern);
    return pse_ok;
}

parse_status parse_pingall(pingall_cmd* cmd) {
    return pse_ok;
}

parse_status get_cmd_info(cmd_enum* cmd, command_u* cmd_info) {
    parse_status status;
    switch (*cmd) {
    case ce_create:
        status = parse_create(&(cmd_info->create));
        break;
    case ce_remove:
        status = parse_remove(&(cmd_info->remove));
        break;
    case ce_exec:
        status = parse_exec(&(cmd_info->exec));
        break;
    case ce_pingall:
        status = parse_pingall(&(cmd_info->pingall));
        break;
    default:
        __builtin_unreachable();
        exit(-1);
    }
    return status;
}
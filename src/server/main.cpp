#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <zmq.h>

extern "C" {

#include "core/defines.h"
#include "utils/logger.h"
#include "server/parsing.h"
#include "server/control_node.h"

}

#include "utils/avl.hpp"
#include <iostream>

int main_repl() {
    parse_status status = pse_ok;
    command_u cmd_info;
    void* result;

    cmd_enum cmd;
    while (status == pse_ok && parse_command(&cmd) == pse_ok) {
        get_cmd_info(&cmd, &cmd_info);
        execute_cmd(&cmd, &cmd_info, &result);
        //print_result(result);
        //sleep(1);
    }
    kill_childs();
    return true;
}

int start() {

    main_repl();

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (!_LOG_INIT())
        exit(-1);
    if (!init_control_node())
        exit(-2);

    start();

    if (!deinit_control_node())
        exit(-2);
    if (!_LOG_DEINIT())
        exit(-1);
}
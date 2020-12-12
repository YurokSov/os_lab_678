#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <zmq.h>

#include "defines.h"
#include "logger.h"
#include "parsing.h"
#include "processor.h"

int main_repl() {
    parse_status status = pse_ok;
    command_u cmd_info;
    void* result;

    cmd_enum cmd;
    while (status == pse_ok && parse_command(&cmd) == pse_ok) {
        get_cmd_info(&cmd, &cmd_info);
        execute_cmd(&cmd, &cmd_info, &result);
        //print_result(result);
    }
    return true;
}

int main(int argc, char* argv[]) {

    main_repl();

    return EXIT_SUCCESS;
}
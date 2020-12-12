#include "computing_node.h"

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

#include "logger.h"

static node_status status;

static void catch_sigterm(int signo) {
    LOG(LL_NOTE, "IN CHILD, KILLED");
    if (status != ns_ok)
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}

node_status node_loop() {
    while (true) {
        LOG(LL_DEBUG, "IN CHILD, AILVE");
        sleep(1);
    }
}

node_status node_start() {
    if (signal(SIGTERM, catch_sigterm) == SIG_ERR) {
        LOG(LL_ERROR, "An error occurred while setting a signal handler.\n");
        return ns_error;
    }
    return node_loop();
}


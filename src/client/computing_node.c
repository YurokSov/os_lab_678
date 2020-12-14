#include "client/computing_node.h"
#include "client/mm_computing_node.h"
#include "utils/logger.h"

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

static node_status status;

static int this_id;

static void catch_sigterm(int signo) {
    LOG(LL_NOTE, "IN CHILD, KILLED");
    if (status != ns_ok)
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}

node_status node_loop() {
    while (true) {
        LOG(LL_DEBUG, "IN CHILD, AILVE");
        sleep(10);
    }
}

node_status node_start(int id, int p_id) {
    this_id = id;

    // if (!_LOG_INIT())
    //    printf("log init error in pid:%d", getpid());
    if (signal(SIGTERM, catch_sigterm) == SIG_ERR) {
        LOG(LL_ERROR, "An error occurred while setting a signal handler.\n");
        return ns_error;
    }

    mm_init_computing_node(id, p_id);

    node_status rv = node_loop();

    mm_deinit_computing_node(id, p_id);

    //if (!_LOG_DEINIT())
    //    printf("log deinit error in pid:%d", getpid());
    return rv;
}


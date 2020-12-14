#include "client/computing_node.h"
#include "client/mm_computing_node.h"
#include "utils/logger.h"

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

static node_status status;

static int this_id;
static int this_p_id;


static void catch_sigterm(int signo) {
    LOG(LL_NOTE, "IN CHILD, KILLED");
    mm_deinit_computing_node(this_id, this_p_id);
    if (status != ns_ok)
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}


void init() {
    if (signal(SIGTERM, catch_sigterm) == SIG_ERR) {
        LOG(LL_ERROR, "An error occurred while setting a signal handler.\n");
        exit(-1);
    }
    mm_init_computing_node(this_id, this_p_id);
}

void fini() {
    mm_deinit_computing_node(this_id, this_p_id);
}


node_status node_loop() {
    int i = 0;
    while (true) {
        if (i == 10) {
            LOG(LL_DEBUG, "IN CHILD, AILVE");
            i = 0;
        }
        mm_recv_command(this_id, this_p_id);
        sleep(1);
        ++i;
    }
}

node_status node_start(int id, int p_id) {
    this_id = id;
    this_p_id = p_id;
    init();
    node_status rv = node_loop();
    fini();
    return rv;
}


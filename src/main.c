#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/syscall.h>

#include <zmq.h>

#include "logger.h"

void __attribute__((destructor)) deinit() {
    if (!_LOG_DEINIT()) {
        return EXIT_FAILURE;
    }
}

void __attribute__((constructor)) init() {
    if (!_LOG_INIT()) {
        return EXIT_FAILURE;
    }
}

int main(int argc, char* argv[]) {



    return EXIT_SUCCESS;
}
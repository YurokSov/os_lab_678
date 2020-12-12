#ifndef INCLUDE_IPC_MUTEX_H
#define INCLUDE_IPC_MUTEX_H

#include <pthread.h>

typedef struct ipc_mutex {
    pthread_mutex_t* obj;
    int is_origin;
    int shared_id;
    char* name;
} ipc_mutex;

ipc_mutex ipc_mutex_init(char* name);

int ipc_mutex_close(ipc_mutex* mutex);

int ipc_mutex_destroy(ipc_mutex* mutex);

#endif
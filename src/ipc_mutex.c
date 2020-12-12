#include "ipc_mutex.h"

#include <errno.h> // errno, ENOENT
#include <fcntl.h> // O_RDWR, O_CREATE
#include <linux/limits.h> // NAME_MAX
#include <sys/mman.h> // shm_open, shm_unlink, mmap, munmap,
// PROT_READ, PROT_WRITE, MAP_SHARED, MAP_FAILED
#include <unistd.h> // ftruncate, close
#include <stdio.h> // perror
#include <stdlib.h> // malloc, free
#include <string.h> // strcpy

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defines.h"

#define MAX_LEN 80

ipc_mutex ipc_mutex_init(char* name) {
    ipc_mutex mutex;
    mutex.is_origin = 0;
    mutex.obj = NULL;
    mutex.name = NULL;
    mutex.shared_id = 0;

    errno = 0;

    mutex.shared_id = shm_open(name, O_RDWR, S_IRWXU | S_IRWXG);
    if (errno == ENOENT) {
        mutex.shared_id = shm_open(name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG);
        mutex.is_origin = 1;
    }
    if (mutex.shared_id == ERROR_CODE) {
        perror("ERROR shm_open");
        return mutex;
    }

    if (ftruncate(mutex.shared_id, sizeof(pthread_mutex_t))) {
        perror("ERROR ftruncate");
        return mutex;
    }

    void* addr = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex.shared_id, 0);
    if (addr == MAP_FAILED) {
        perror("ERROR mmap");
        return mutex;
    }
    pthread_mutex_t* mutex_ptr = (pthread_mutex_t*)addr;

    if (mutex.is_origin) {
        pthread_mutexattr_t attr;
        if (pthread_mutexattr_init(&attr)) {
            perror("ERROR pthread_mutexattr_init");
            return mutex;
        }
        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
            perror("ERROR pthread_mutexattr_setpshared");
            return mutex;
        }
        if (pthread_mutex_init(mutex_ptr, &attr)) {
            perror("ERROR pthread_mutex_init");
            return mutex;
        }
    }
    mutex.obj = mutex_ptr;
    mutex.name = (char*)malloc(MAX_LEN + 1);
    strcpy(mutex.name, name);
    return mutex;
}

int ipc_mutex_close(ipc_mutex* mutex) {

    if (munmap((void*)mutex->obj, sizeof(pthread_mutex_t))) {
        perror("ERROR munmap");
        return ERROR_CODE;
    }

    mutex->obj = NULL;
    if (close(mutex->shared_id)) {
        perror("ERROR close");
        return ERROR_CODE;
    }

    mutex->shared_id = 0;
    free(mutex->name);
    return 0;
}

int ipc_mutex_destroy(ipc_mutex* mutex) {

    if ((errno = pthread_mutex_destroy(mutex->obj))) {
        perror("ERROR pthread_mutex_destroy");
        return ERROR_CODE;
    }
    if (munmap((void*)mutex->obj, sizeof(pthread_mutex_t))) {
        perror("ERROR munmap");
        return ERROR_CODE;
    }

    mutex->obj = NULL;
    if (close(mutex->shared_id)) {
        perror("ERROR close");
        return ERROR_CODE;
    }

    mutex->shared_id = 0;
    if (shm_unlink(mutex->name)) {
        perror("ERROR shm_unlink");
        return -1;
    }

    free(mutex->name);
    return 0;
}
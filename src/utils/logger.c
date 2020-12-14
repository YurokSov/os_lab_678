#include "utils/logger.h"

#include "core/ipc_mutex.h"

//static pthread_mutex_t _log_mutex;
static ipc_mutex _log_mutex;

/**
 * @warning не потокобезопасный, только под unix
 */
QWORD _GET_TIME_STAMP() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    //return tv.tv_sec * (QWORD)1000000 + tv.tv_usec;
    return tv.tv_sec * (QWORD)1000000 + tv.tv_usec;
}

/**
 * @warning не потокобезопасный
 */
char* _GET_DATETIME() {
    char* buffer = malloc(_LOG_BUF_SIZE * sizeof(char));
    time_t now = time(0);

    strftime(buffer, _LOG_BUF_SIZE, "%Y-%m-%d.%X", localtime(&now));
    sprintf(buffer + 19, "::%lld", _GET_TIME_STAMP());
    return buffer;
}

pid_t _GET_PID() {
    pid_t pid = getpid();
    return pid;
}

bool _LOG_INIT() {

    _log_mutex = ipc_mutex_init("/log-mutex");
    if (_log_mutex.obj == NULL) {
        return false;
    }
    //if (_log_mutex.is_origin) {
    //    printf("LOGGER: mutex origin in %d\n", getpid());
    //}
    // if (pthread_mutex_init(&_log_mutex, NULL))
    // {
    //     fprintf(stderr, "_LOG_INIT: mutex init failed\n");
    //     perror("ERROR");
    //     return false;
    // }
    return true;
}

bool _LOG_DEINIT() {
    if (ipc_mutex_destroy(&_log_mutex)) {
        return false;
    }
    // if (pthread_mutex_destroy(&_log_mutex)) {
    //     fprintf(stderr, "_LOG_DEINIT: mutex destroy failed\n");
    //     perror("ERROR");
    //     return false;
    // }
    return true;
}

// bool _LOG_FINAL() {
//     if (ipc_mutex_destroy(&_log_mutex)) {
//         return false;
//     }
//     return true;
// }

void _LOG_WRITE(int level, const char* file, int line, const char* format, ...) {
    pthread_mutex_lock(_log_mutex.obj);

    if (level <= UD_LOG_LEVEL) {
        va_list args;
        va_start(args, format);

        char* buffer = _GET_DATETIME();

        switch (level) {
        case LL_FATAL:
            fprintf(UD_LOG_FILE, "[FATAL]");
            break;
        case LL_ERROR:
            fprintf(UD_LOG_FILE, "[ERROR]");
            break;
        case LL_WARNING:
            fprintf(UD_LOG_FILE, "[WARNING]");
            break;
        case LL_NOTE:
            fprintf(UD_LOG_FILE, "[NOTE]");
            break;
        case LL_DEBUG:
            fprintf(UD_LOG_FILE, "[DEBUG]");
            break;
        default:
            fprintf(UD_LOG_FILE, "[UNKNOWN]");
            break;
        }

        fprintf(UD_LOG_FILE, "[%s PID:%d][%s (%d)]: ", buffer, _GET_PID(), file, line);
        vfprintf(UD_LOG_FILE, format, args);
        fprintf(UD_LOG_FILE, "\n");
        fflush(UD_LOG_FILE);
        free(buffer);
        va_end(args);
    }

    pthread_mutex_unlock(_log_mutex.obj);
}
#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/time.h>

#include "defines.h"

/**
 * @brief макрос лог
 */
#define LOG(level, format, ...) _LOG_WRITE(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LL_DEBUG 4
#define LL_NOTE 3
#define LL_WARNING 2
#define LL_ERROR 1
#define LL_FATAL 0

#ifndef UD_LOG_LEVEL
#define UD_LOG_LEVEL LL_DEBUG
#endif

#ifndef UD_LOG_FILE
#define UD_LOG_FILE stdout
#endif

#define _LOG_BUF_SIZE 80

QWORD _GET_TIME_STAMP();

char* _GET_DATETIME();

pid_t _GET_PID();

bool _LOG_INIT();

bool _LOG_DEINIT();

void _LOG_WRITE(int level, const char* file, int line, const char* format, ...);

#endif
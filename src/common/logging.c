#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>

static struct {
    LogLevel level;
    int destinations;
    FILE* log_file;
    char* app_name;
    pthread_mutex_t mutex;
    bool initialized;
} log_ctx = {
    .level = LOG_LEVEL_DEBUG,
    .destinations = LOG_DEST_STDERR,
    .log_file = NULL,
    .app_name = NULL,
    .initialized = false
};

static const char* level_strings[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE"
};

static int level_to_syslog[] = {
    LOG_ERR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_DEBUG
};

int log_init(const char* app_name, LogLevel level, int destinations) {
    if (log_ctx.initialized) {
        return 0;  // Already initialized
    }

    if (pthread_mutex_init(&log_ctx.mutex, NULL) != 0) {
        return errno;
    }

    log_ctx.level = level;
    log_ctx.destinations = destinations;
    
    if (app_name) {
        log_ctx.app_name = strdup(app_name);
    }

    if (destinations & LOG_DEST_SYSLOG) {
        openlog(app_name, LOG_PID | LOG_NDELAY, LOG_USER);
    }

    log_ctx.initialized = true;
    return 0;
}

int log_set_file(const char* filepath) {
    if (!filepath) {
        return EINVAL;
    }

    pthread_mutex_lock(&log_ctx.mutex);

    if (log_ctx.log_file) {
        fclose(log_ctx.log_file);
        log_ctx.log_file = NULL;
    }

    log_ctx.log_file = fopen(filepath, "a");
    if (!log_ctx.log_file) {
        int err = errno;
        pthread_mutex_unlock(&log_ctx.mutex);
        return err;
    }

    // Enable line buffering for the log file
    setvbuf(log_ctx.log_file, NULL, _IOLBF, 0);

    pthread_mutex_unlock(&log_ctx.mutex);
    return 0;
}

void log_set_level(LogLevel level) {
    pthread_mutex_lock(&log_ctx.mutex);
    log_ctx.level = level;
    pthread_mutex_unlock(&log_ctx.mutex);
}

void _log_write(LogLevel level, const char* file, const char* func, 
                int line, const char* fmt, ...) {
    if (!log_ctx.initialized || level > log_ctx.level) {
        return;
    }

    pthread_mutex_lock(&log_ctx.mutex);

    // Get current time with microsecond precision
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    struct tm* tm_info = localtime(&tv.tv_sec);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Format the message
    va_list args;
    va_start(args, fmt);
    char msg_buf[4096];
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    // Write to stderr if enabled
    if (log_ctx.destinations & LOG_DEST_STDERR) {
        fprintf(stderr, "%s.%06ld [%s] %s:%d %s(): %s\n",
                timestamp, tv.tv_usec, level_strings[level],
                file, line, func, msg_buf);
        fflush(stderr);
    }

    // Write to file if enabled
    if ((log_ctx.destinations & LOG_DEST_FILE) && log_ctx.log_file) {
        fprintf(log_ctx.log_file, "%s.%06ld [%s] %s:%d %s(): %s\n",
                timestamp, tv.tv_usec, level_strings[level],
                file, line, func, msg_buf);
        fflush(log_ctx.log_file);
    }

    // Write to syslog if enabled
    if (log_ctx.destinations & LOG_DEST_SYSLOG) {
        syslog(level_to_syslog[level], "[%s] %s:%d %s(): %s",
               level_strings[level], file, line, func, msg_buf);
    }

    pthread_mutex_unlock(&log_ctx.mutex);
}

void log_cleanup(void) {
    if (!log_ctx.initialized) {
        return;
    }

    pthread_mutex_lock(&log_ctx.mutex);

    if (log_ctx.destinations & LOG_DEST_SYSLOG) {
        closelog();
    }

    if (log_ctx.log_file) {
        fclose(log_ctx.log_file);
        log_ctx.log_file = NULL;
    }

    free(log_ctx.app_name);
    log_ctx.app_name = NULL;

    pthread_mutex_unlock(&log_ctx.mutex);
    pthread_mutex_destroy(&log_ctx.mutex);

    log_ctx.initialized = false;
} 
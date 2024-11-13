#ifndef SEARCH_ENGINE_LOGGING_H
#define SEARCH_ENGINE_LOGGING_H

#include <stdarg.h>
#include <stdbool.h>

// Log levels
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_TRACE = 4
} LogLevel;

// Log destinations
typedef enum {
    LOG_DEST_STDERR = 1,    // Log to stderr
    LOG_DEST_SYSLOG = 2,    // Log to syslog
    LOG_DEST_FILE = 4       // Log to file
} LogDestination;

// Initialize logging system
int log_init(const char* app_name, LogLevel level, int destinations);

// Set log file path (if LOG_DEST_FILE is used)
int log_set_file(const char* filepath);

// Set maximum log level
void log_set_level(LogLevel level);

// Internal logging function - don't call directly, use macros
void _log_write(LogLevel level, const char* file, const char* func, 
                int line, const char* fmt, ...);

// Cleanup
void log_cleanup(void);

// Convenience macros for debug builds
#ifdef DEBUG
    #define DEBUG_LOG(...) _log_write(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, __VA_ARGS__)
    #define TRACE_LOG(...) _log_write(LOG_LEVEL_TRACE, __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
    #define DEBUG_LOG(...) ((void)0)
    #define TRACE_LOG(...) ((void)0)
#endif

// Always enabled macros
#define ERROR_LOG(...) _log_write(LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define WARN_LOG(...)  _log_write(LOG_LEVEL_WARN,  __FILE__, __func__, __LINE__, __VA_ARGS__)
#define INFO_LOG(...)  _log_write(LOG_LEVEL_INFO,  __FILE__, __func__, __LINE__, __VA_ARGS__)

#endif // SEARCH_ENGINE_LOGGING_H 
// logger.c
#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

const char* logLevelToString(LogLevel level) {
    switch(level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

LogLevel globalLogLevel = LOG_INFO;

void log_message(LogLevel level, const char *format, ...) {
    if (level < globalLogLevel) return;

    va_list args;
    va_start(args, format);

    FILE *file = fopen("application.log", "a");
    if (file == NULL) {
        perror("Error opening log file");
        va_end(args);
        return;
    }

    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    time_str[24] = '\0'; // Remove newline at end

    fprintf(file, "[%s] %s: ", time_str, logLevelToString(level));
    vfprintf(file, format, args); // Use vfprintf for variable arguments
    fprintf(file, "\n");

    va_end(args);
    fclose(file);
}

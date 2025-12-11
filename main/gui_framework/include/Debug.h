#ifndef DEBUG_H
#define DEBUG_H

#define GUI_TRACE_ENABLED 1

#if GUI_TRACE_ENABLED
#include <stdio.h>
#define GUI_TRACE(function_name, format, ...)                      \
    fprintf(stderr, "[GUI TRACE] %s: " format "\n", function_name, \
            ##__VA_ARGS__)

#else
#define GUI_TRACE(function_name, format, ...) \
    do {                                      \
    } while (0)

#endif  // GUI_TRACE_ENABLED
#endif  // DEBUG_H
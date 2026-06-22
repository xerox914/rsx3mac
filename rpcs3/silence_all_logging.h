#pragma once

// Disable all global logging macros
#define LOG_NOTICE(...)   do {} while (0)
#define LOG_WARNING(...)  do {} while (0)
#define LOG_ERROR(...)    do {} while (0)
#define LOG_TRACE(...)    do {} while (0)
#define LOG_FATAL(...)    do {} while (0)

// Disable RSX logger wrappers
#define rsx_log_notice(...)   do {} while (0)
#define rsx_log_warning(...)  do {} while (0)
#define rsx_log_error(...)    do {} while (0)
#define rsx_log_trace(...)    do {} while (0)


#pragma once
#include <stdarg.h>
#include <stdint.h>

#ifdef __cpluscplus
extern "C"
{
#endif
    static const unsigned t1net_log_max_len = 512;
    /// log message severity levels.
    enum e_severity {s_trace, s_debug, s_info, s_warning, s_error, s_none_last};

    typedef void (*log_function_t)(e_severity level, unsigned length, const char* message);

    log_function_t t1net_log_function_get();
    void t1net_log_function_set(log_function_t);

    void log_write(e_severity level, const char* format, ...);
#ifdef __cpluscplus
}
#endif

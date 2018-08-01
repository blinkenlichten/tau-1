#include <t1net/log.h>
#include <cstring>
#include <array>
#include <functional>

namespace tau1
{
namespace log
{
namespace
{
log_function_t g_logger = nullptr;

std::array<char, t1net_log_max_len> g_buffer = {};
using fn_log_function = std::function<void(e_severity level, size_t length, const char* message)>;
fn_log_function g_logger_wrapper = [](e_severity level, size_t length, const char* message)
{
    if (!g_logger)
        return;
    g_logger(level, length, message);
};

void set(fn_log_function function)
{
    g_logger_wrapper = function;
}

const fn_log_function& get()
{
    return g_logger_wrapper;
}

}//anonymous namespace
}//log namespace
}//tau1 namespace

static void wrapper_invoking_func(e_severity level, unsigned length, const char* message)
{
    using namespace tau1::log;
    if (g_logger_wrapper)
        g_logger_wrapper(level, length, message);
}

log_function_t t1net_log_function_get()
{
    //return what was set last time: C function of C++ std::function wrapper.
    using namespace tau1::log;
    return g_logger? g_logger : &wrapper_invoking_func;
}

void t1net_log_function_set(log_function_t function)
{
    using namespace tau1::log;
    g_logger = function;
    g_logger_wrapper = [](e_severity level, size_t length, const char* message)
        {
            if (!g_logger)
                return;
            g_logger(level, length, message);
        };
}

void log_write(e_severity level, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    int written = vsnprintf(tau1::log::g_buffer.data(), tau1::log::g_buffer.size(), format, va);
    va_end(va);
    if (0 < written)
    {
        auto func_ptr = t1net_log_function_get();
        func_ptr(level, (size_t)written, tau1::log::g_buffer.data());
    }
}

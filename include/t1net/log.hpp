#pragma once
#include "log.h"

namespace tau1
{
namespace log
{
using fn_log_function = std::function<void(e_severity level, size_t length, const char* message)>;
/// Set logging function for the module.
/// @param function: shall not be nullptr.
void set(fn_log_function function);

/// Get logging function for the module.
/// @param function: shall not be nullptr.
const fn_log_function& get();

}//log
}//tau1

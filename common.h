#pragma once

// These are not coming from stdarg C standard library,
// are instead provided as builtin by clang compiler.
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

void printf(const char *fmt, ...);

#pragma once

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

// References to the symbols created in kernel.ld
// The "[]" is used as we need the start address of these values, omitting it
// will give us the value at the 0th byte of that section.
extern char __bss[], __bss_end[], __stack_top[];

// These are not coming from stdarg C standard library,
// are instead provided as builtin by clang compiler.
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

struct sbiret {
  long error;
  long value;
};

void *memset(void *buf, char c, size_t n);
void putchar(char ch);
void printf(const char *fmt, ...);

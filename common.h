#pragma once

typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t paddr_t; // Physical address type
typedef uint32_t vaddr_t; // Virtual address type, equivalent to uintptr_t in stdlib

#define true  1
#define false 0
#define NULL  ((void *) 0)

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
#define align_up(value, align)   __builtin_align_up(value, align) // align must be power of 2
#define is_aligned(value, align) __builtin_is_aligned(value, align) // align must be power of 2 
#define offsetof(type, member)   __builtin_offsetof(type, member) // Get offset of member in a struct

struct sbiret {
  long error;
  long value;
};

void *memset(void *buf, char c, size_t n);
void putchar(char ch);
void printf(const char *fmt, ...);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);

#pragma once

// Note "##" removes the preceding "," where there's only 1 arg.
// __VA_ARGS__ is supported by the compiler extension for macros with
// variable number of args.
#define PANIC(fmt, ...)                                                        \
  do {                                                                         \
    printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);      \
    while (1) {                                                                \
    }                                                                          \
  } while (0)

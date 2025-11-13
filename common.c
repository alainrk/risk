#include "common.h"

// SBI is "API for OS", this function just does the heavy lift to call its
// interface through ASM. To call the SBI an ECALL is needed.
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {
  register long a0 __asm__("a0") = arg0; // Returned by SBI will contain error.
  register long a1 __asm__("a1") =
      arg1; // Returned by SBI will contain result value.
  register long a2 __asm__("a2") =
      arg2; // These must be preserved across SBI call/callee
  register long a3 __asm__("a3") = arg3; // |
  register long a4 __asm__("a4") = arg4; // |
  register long a5 __asm__("a5") = arg5; // |__________
  register long a6 __asm__("a6") = fid;  // SBI Extension ID
  register long a7 __asm__("a7") = eid;  // SBI Function ID

  // ECALL is used as control transfer instruction between supervisor and SEE.
  __asm__ __volatile__("ecall"
                       : "=r"(a0), "=r"(a1)
                       : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                         "r"(a6), "r"(a7)
                       : "memory");
  return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
  sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

void *memset(void *buf, char c, size_t n) {
  uint8_t *p = (uint8_t *)buf;
  while (n--)
    *p++ = c;
  return buf;
}

void printf(const char *fmt, ...) {
  va_list vargs;
  va_start(vargs, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++; // Skip '%'
      switch (*fmt) {
      case '\0': // '%' at the end of the format string
        putchar('%');
        goto end;
      case '%': // Escaping "%%", print "%"
        putchar('%');
        break;
      case 's': { // NULL-terminated string
        const char *s = va_arg(vargs, const char *);
        while (*s) {
          putchar(*s);
          s++;
        }
        break;
      }
      case 'd': {
        int value = va_arg(vargs, int);
        unsigned magnitude = value; // unsigned to avoid INT_MIN issue
        if (value < 0) {
          putchar('-');
          magnitude = -magnitude;
        }

        unsigned divisor = 1;
        while (magnitude / divisor > 9)
          divisor *= 10;

        while (divisor > 0) {
          putchar('0' + magnitude / divisor);
          magnitude %= divisor;
          divisor /= 10;
        }

        break;
      }
      case 'x': { // Exadecimal
        unsigned value = va_arg(vargs, unsigned);
        for (int i = 7; i >= 0; i--) {
          // Get 4 bits at a time (hex representation),
          // shifting value to right, then mask to keep
          // the last 4 bits everytime (0xf).
          unsigned nibble = (value >> (i * 4)) & 0xf;
          // Get the corresponding hex digit and print it
          putchar("0123456789abcdef"[nibble]);
        }
      }
      }
    } else {
      putchar(*fmt);
    }

    fmt++;
  }

end:
  va_end(vargs);
}

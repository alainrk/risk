#include "kernel.h"
#include "common.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

// References to the symbols created in kernel.ld
// The "[]" is used as we need the start address of these values, omitting it
// will give us the value at the 0th byte of that section.
extern char __bss[], __bss_end[], __stack_top[];

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

void kernel_main(void) {
  printf("\nHello %s! - %d + %d = %x\n", "world", 20, 22, 20 + 22);

  // Sometimes the bss section is initialized at 0 by the bootloader, but it's
  // better to make sure it's done.
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);
  // Infinite kernel loop.
  for (;;)
    __asm__ __volatile__("wfi");
}

// As specified in kerel.ld boot() is the entry point of the kernel after boot.
// __volatile__ avoids the compiler optimizing the function.
// Attributes:
//    - section(".text.boot"): placement of the function in the linker script
//    (0x80200000, where OpenSBI will jump to, without knowing anything the
//    entry point)
//    - naked: asks the compiler not to generate any unnecessary code before and
//    after the asm inline function body (e.g. return)
__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
  __asm__ __volatile__(
      "mv sp, %[stack_top]\n" // Set the stack pointer to the end of the stack
                              // area (grows backwards to 0).
      "j kernel_main\n"       // Jump to the kernel main function
      :
      : [stack_top] "r"(
          __stack_top) // Pass the stack top address as %[stack_top]
  );
}

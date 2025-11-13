#include "kernel.h"
#include "common.h"

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

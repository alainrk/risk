typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

// References to the symbols created in kernel.ld
extern char __bss[], __bss_end[], __stack_top[];

void *memset(void *buf, char c, size_t n) {
  uint8_t *p = (uint8_t *)buf;
  while (n--)
    *p++ = c;
  return buf;
}

void kernel_main(void) {
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  for (;;)
    ;
}

// As specified in kerel.ld boot() is the entry point of the kernel after boot
__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
  // __volatile__ avoids the compiler optimizing this stuff
  __asm__ __volatile__(
      "mv sp, %[stack_top]\n" // Set the stack pointer to the end of the stack
                              // area (grows backwards to 0).
      "j kernel_main\n"       // Jump to the kernel main function
      :
      : [stack_top] "r"(
          __stack_top) // Pass the stack top address as %[stack_top]
  );
}

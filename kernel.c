#include "kernel.h"
#include "common.h"

struct process procs[PROCS_MAX]; // Process table
struct process *current_proc;    // Currently running process
struct process *idle_proc; // Idle process to run when there's no other runnable
                           // process available

// Bump (or linear) allocator
paddr_t alloc_pages(uint32_t n) {
  // Only init at the beginning at the start of the available ram region.
  static paddr_t next_paddr = (paddr_t)__free_ram;
  paddr_t paddr = next_paddr;
  next_paddr += n * PAGE_SIZE;

  if (next_paddr > (paddr_t)__free_ram_end)
    PANIC("out of memory");

  memset((void *)paddr, 0, n * PAGE_SIZE);
  return paddr;
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

void handle_trap(struct trap_frame *f) {
  (void)f;
  uint32_t scause = READ_CSR(scause);
  uint32_t stval = READ_CSR(stval);
  uint32_t user_pc = READ_CSR(sepc);

  PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval,
        user_pc);
}

// Exception handler definition.
// Entry point of the exception handler to be stores in stvec (Supervisor Trap
// Vector Base Address Register).
//
// - sscratch is used as a temporary storage to save the stack pointer at the
// time of exception occurrence, which is later restored.
// - Floating-point registers are not used within the kernel, and thus there's
// no need to save them here. Generally, they are saved and restored during
// thread switching.
// - The stack pointer is set in the a0 register, and the handle_trap function
// is called. SP now contains register values stored in the same structure as
// the trap_frame.
// - __attribute__((aligned(4))) aligns the function's starting address to a
// 4-byte boundary, since stvec not only holds the address of the exception
// handler but also has flags representing the mode in its lower 2 bits.
//
// NOTE: The entry point of exception handlers is critical and error-prone.
// The original values of general-purpose registers are saved onto the stack,
// even SP by using sscratch. Accidentally overwriting the a0 register can lead
// to hard-to-debug problems like "local variable values change for no apparent
// reason". Saving the program state correctly is critical!
// ----------------------------------------------------------------------------
__attribute__((naked)) __attribute__((aligned(4))) void kernel_entry(void) {
  __asm__ __volatile__(
      // Retrieve the kernel stack of the running process from sscratch (see
      // yield()).
      // This is basically a swap operation:
      //    tmp = sp;
      //    sp = sscratch;
      //    sscratch = tmp;
      "csrrw sp, sscratch, sp\n"

      "addi sp, sp, -4 * 31\n"
      "sw ra,  4 * 0(sp)\n"
      "sw gp,  4 * 1(sp)\n"
      "sw tp,  4 * 2(sp)\n"
      "sw t0,  4 * 3(sp)\n"
      "sw t1,  4 * 4(sp)\n"
      "sw t2,  4 * 5(sp)\n"
      "sw t3,  4 * 6(sp)\n"
      "sw t4,  4 * 7(sp)\n"
      "sw t5,  4 * 8(sp)\n"
      "sw t6,  4 * 9(sp)\n"
      "sw a0,  4 * 10(sp)\n"
      "sw a1,  4 * 11(sp)\n"
      "sw a2,  4 * 12(sp)\n"
      "sw a3,  4 * 13(sp)\n"
      "sw a4,  4 * 14(sp)\n"
      "sw a5,  4 * 15(sp)\n"
      "sw a6,  4 * 16(sp)\n"
      "sw a7,  4 * 17(sp)\n"
      "sw s0,  4 * 18(sp)\n"
      "sw s1,  4 * 19(sp)\n"
      "sw s2,  4 * 20(sp)\n"
      "sw s3,  4 * 21(sp)\n"
      "sw s4,  4 * 22(sp)\n"
      "sw s5,  4 * 23(sp)\n"
      "sw s6,  4 * 24(sp)\n"
      "sw s7,  4 * 25(sp)\n"
      "sw s8,  4 * 26(sp)\n"
      "sw s9,  4 * 27(sp)\n"
      "sw s10, 4 * 28(sp)\n"
      "sw s11, 4 * 29(sp)\n"

      // Retrieve and save the sp at the time of exception.
      "csrr a0, sscratch\n"
      "sw a0,  4 * 30(sp)\n"

      // Reset the kernel stack.
      "addi a0, sp, 4 * 31\n"
      "csrw sscratch, a0\n"

      "mv a0, sp\n"
      "call handle_trap\n"

      "lw ra,  4 * 0(sp)\n"
      "lw gp,  4 * 1(sp)\n"
      "lw tp,  4 * 2(sp)\n"
      "lw t0,  4 * 3(sp)\n"
      "lw t1,  4 * 4(sp)\n"
      "lw t2,  4 * 5(sp)\n"
      "lw t3,  4 * 6(sp)\n"
      "lw t4,  4 * 7(sp)\n"
      "lw t5,  4 * 8(sp)\n"
      "lw t6,  4 * 9(sp)\n"
      "lw a0,  4 * 10(sp)\n"
      "lw a1,  4 * 11(sp)\n"
      "lw a2,  4 * 12(sp)\n"
      "lw a3,  4 * 13(sp)\n"
      "lw a4,  4 * 14(sp)\n"
      "lw a5,  4 * 15(sp)\n"
      "lw a6,  4 * 16(sp)\n"
      "lw a7,  4 * 17(sp)\n"
      "lw s0,  4 * 18(sp)\n"
      "lw s1,  4 * 19(sp)\n"
      "lw s2,  4 * 20(sp)\n"
      "lw s3,  4 * 21(sp)\n"
      "lw s4,  4 * 22(sp)\n"
      "lw s5,  4 * 23(sp)\n"
      "lw s6,  4 * 24(sp)\n"
      "lw s7,  4 * 25(sp)\n"
      "lw s8,  4 * 26(sp)\n"
      "lw s9,  4 * 27(sp)\n"
      "lw s10, 4 * 28(sp)\n"
      "lw s11, 4 * 29(sp)\n"
      "lw sp,  4 * 30(sp)\n"
      "sret\n");
}

__attribute__((naked)) void switch_context(uint32_t *prev_sp,
                                           uint32_t *next_sp) {
  __asm__ __volatile__(
      // Save callee-saved registers onto the current process's stack.
      "addi sp, sp, -13 * 4\n" // Allocate stack space for 13 4-byte registers
      "sw ra,  0  * 4(sp)\n"   // Save callee-saved registers only
      "sw s0,  1  * 4(sp)\n"
      "sw s1,  2  * 4(sp)\n"
      "sw s2,  3  * 4(sp)\n"
      "sw s3,  4  * 4(sp)\n"
      "sw s4,  5  * 4(sp)\n"
      "sw s5,  6  * 4(sp)\n"
      "sw s6,  7  * 4(sp)\n"
      "sw s7,  8  * 4(sp)\n"
      "sw s8,  9  * 4(sp)\n"
      "sw s9,  10 * 4(sp)\n"
      "sw s10, 11 * 4(sp)\n"
      "sw s11, 12 * 4(sp)\n"

      // Switch the stack pointer.
      "sw sp, (a0)\n" // *prev_sp = sp;
      "lw sp, (a1)\n" // Switch stack pointer (sp) here

      // Restore callee-saved registers from the next process's stack.
      "lw ra,  0  * 4(sp)\n" // Restore callee-saved registers only
      "lw s0,  1  * 4(sp)\n"
      "lw s1,  2  * 4(sp)\n"
      "lw s2,  3  * 4(sp)\n"
      "lw s3,  4  * 4(sp)\n"
      "lw s4,  5  * 4(sp)\n"
      "lw s5,  6  * 4(sp)\n"
      "lw s6,  7  * 4(sp)\n"
      "lw s7,  8  * 4(sp)\n"
      "lw s8,  9  * 4(sp)\n"
      "lw s9,  10 * 4(sp)\n"
      "lw s10, 11 * 4(sp)\n"
      "lw s11, 12 * 4(sp)\n"
      "addi sp, sp, 13 * 4\n" // Pop the 13 4-byte registers from the stack
      "ret\n");
}

struct process *create_process(uint32_t pc) {
  // Find the first unused process control structure.
  struct process *proc = NULL;

  int i;
  for (i = 0; i < PROCS_MAX; i++) {
    if (procs[i].state == PROC_UNUSED) {
      proc = &procs[i];
      break;
    }
  }

  if (!proc)
    PANIC("no free process slots");

  // Stack callee-saved registers.
  // These register values will be restored in the first context switch.
  uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)];
  *--sp = 0;            // s11
  *--sp = 0;            // s10
  *--sp = 0;            // s9
  *--sp = 0;            // s8
  *--sp = 0;            // s7
  *--sp = 0;            // s6
  *--sp = 0;            // s5
  *--sp = 0;            // s4
  *--sp = 0;            // s3
  *--sp = 0;            // s2
  *--sp = 0;            // s1
  *--sp = 0;            // s0
  *--sp = (uint32_t)pc; // ra

  proc->pid = i + 1;
  proc->state = PROC_RUNNABLE;
  proc->sp = (uint32_t)sp;

  return proc;
}

void yield(void) {
  struct process *next = idle_proc;
  for (int i = 0; i < PROCS_MAX; i++) {
    struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
    if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
      next = proc;
      break;
    }
  }

  // No new process to run
  if (next == current_proc)
    return;

  // We need to do this because we use separate kernel stacks for each
  // process. So we pre-load the "emergency backup stack pointer" into a safe
  // register so that if an interrupt fires, the kernel immediately knows where
  // its valid memory is located.
  __asm__ __volatile__(
      "csrw sscratch, %[sscratch]\n"
      :
      : [sscratch] "r"((uint32_t)&next->stack[sizeof(next->stack)]));

  // Context switch
  struct process *prev = current_proc;
  current_proc = next;
  switch_context(&prev->sp, &next->sp);
}

void delay(int sleep) {
  for (int i = 0; i < sleep; i++)
    __asm__ __volatile__("nop");
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void) {
  printf("starting process A\n");
  while (1) {
    putchar('A');
    delay(500000000);
    yield();
  }
}

void proc_b_entry(void) {
  printf("starting process B\n");
  while (1) {
    putchar('B');
    delay(500000000);
    yield();
  }
}

void kernel_main(void) {
  printf("\nRisk Kernel started...\n");

  // Sometimes the bss section is initialized at 0 by the bootloader, but it's
  // better to make sure it's done.
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  // Tells the CPU where the exception handler is located.
  WRITE_CSR(stvec, (uint32_t)kernel_entry);

  // Create the idle process to be run every time there's no other runnable
  // process.
  idle_proc = create_process((uint32_t)NULL);
  idle_proc->pid = 0;
  current_proc = idle_proc;

  // Create two test process
  proc_a = create_process((uint32_t)proc_a_entry);
  proc_b = create_process((uint32_t)proc_b_entry);

  // Run!
  yield();
  PANIC("Yield to idle process");

  // Infinite kernel loop.
  for (;;)
    __asm__ __volatile__("wfi");
}

// void test(void) {
// Test printf
// printf("\nHello %s! - %d + %d = %x\n", "world", 20, 22, 20 + 22);
// printf("%s cmp %s = %d\n", "Hello", "Hello", strcmp("Hello", "Hello"));
// printf("%s cmp %s = %d\n", "Hello", "World", strcmp("Hello", "World"));
// printf("%s cmp %s = %d\n", "World", "Hello", strcmp("World", "Hello"));
// char *s = "Test this and that.";
// char *p = "                   ";
// p = strcpy(p, s);
// printf("Copied s (%s) into p (%s)\n", s, p);

// Test pages allocation
// paddr_t paddr0 = alloc_pages(1);
// paddr_t paddr1 = alloc_pages(2);
// paddr_t paddr2 = alloc_pages(3);
// printf("alloc_pages test: paddr0=%x\n", paddr0);
// printf("alloc_pages test: paddr1=%x\n", paddr1);
// printf("alloc_pages test: paddr2=%x\n", paddr2);

// Test PANIC
// Illegal "pseudo-instruction" translated into:
// `csrrw x0, cycle, x0`
// Where cycle is readonly so triggers an exception.
// __asm__ __volatile__("unimp");
// }

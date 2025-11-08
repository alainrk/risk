/* Memory layout definition of executables.
 * Based on this layout the linker will assign memory
 * addresses to different sections of the program (functions, variables).
 *
 * Note: "." is the current address, it increments while adding sections.
 */


/* Kernel entry point */
ENTRY(boot)

SECTIONS {
    /* Base address*/
    . = 0x80200000;

    /* Code section */
    .text :{
        KEEP(*(.text.boot));
        /* Any .text and ".text."-starting section will be here */
        *(.text .text.*);
    }

    /* Read-only data sections */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    /* Read/Write data sections */
    .data : ALIGN(4) {
        *(.data .data.*);
    }

    /* Read/Write data sections 0-initialized */
    .bss : ALIGN(4) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    /* Reserve stack space right after bss */
    /* Ensures current address will be 4-byte aligned, before next directive. */
    . = ALIGN(4);
    /* 128KB */
    . += 128 * 1024;
    __stack_top = .;
}

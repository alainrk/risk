#!/bin/bash
set -xue

# QEMU file path
QEMU=qemu-system-riscv32

# Ubuntu users: use CC=clang
CC=/opt/homebrew/opt/llvm/bin/clang

# Build the kernel
#   -g3 Generate the maximum amount of debug information.
#   --target=riscv32-unknown-elf Compile for 32-bit RISC-V.
#   -ffreestanding Do not use the standard library of the host environment (your development environment).
#   -fuse-ld=lld Use LLVM linker
#   -fno-stack-protector Disable unnecessary stack protection to avoid unexpected behavior in stack manipulation.
#   -nostdlib Do not link the standard library.
#   -Wl,-Tkernel.ld Specify the linker script.
#   -Wl,-Map=kernel.map Output a map file (linker allocation result).
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib"
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf common.c kernel.c

# Start QEMU:
#   -machine virt       Start a virt machine.
#                       Check other supported machines with the -machine '?' option.
#   -bios default       Use the default firmware (OpenSBI here).
#   -nographic          Start QEMU without a GUI window.
#   -serial mon:stdio   Connect QEMU's standard i/o to the virtual machine's serial port.
#                       Specifying `mon:` allows switching to the QEMU monitor by pressing Ctrl+A then C.
#   --no-reboot         If the virtual machine crashes, stop the emulator without rebooting (useful for debugging).
$QEMU -machine virt \
	-bios default \
	-nographic \
	-serial mon:stdio \
	--no-reboot \
	-kernel kernel.elf

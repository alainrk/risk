# risk

RISC-V microkernel

## Toolchain

**MacOS**

```sh
brew install llvm lld qemu

$ export PATH="$PATH:$(brew --prefix)/opt/llvm/bin"
$ which llvm-objcopy
/opt/homebrew/opt/llvm/bin/llvm-objcopy
```

**Ubuntu**

```sh
sudo apt update && sudo apt install -y clang llvm lld qemu-system-riscv32 curl
```

### Download OpenSBI firmware

```sh
curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin
```

## Qemu

```sh
C-A c # switch to QEMU console
```

## References

- [OS in 1k Lines](https://operating-system-in-1000-lines.vercel.app/en/)
- [RISC-V Tech Spec](https://riscv.atlassian.net/wiki/spaces/HOME/pages/16154769/RISC-V+Technical+Specifications#ISA)
- [RISC-V Unprivileged ISA](https://docs.riscv.org/reference/isa/_attachments/riscv-unprivileged.pdf)
- [RISC-V Privileged ISA](https://docs.riscv.org/reference/isa/_attachments/riscv-privileged.pdf)
- [RVemu](https://github.com/d0iasm/rvemu)
- [Extended ASM GCC](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)
- [Example ASM 1](https://github.com/nuta/microkernel-book/blob/52d66bd58cd95424f009e2df8bc1184f6ffd9395/kernel/riscv32/asm.h)
- [Example ASM 2](https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/riscv.h)
- [OpenSBI](https://github.com/riscv-software-src/opensbi)

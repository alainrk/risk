run:
	./run.sh

disasm:
	llvm-objdump -d kernel.elf

check-addr:
	llvm-nm kernel.elf

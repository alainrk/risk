run:
	./run.sh

disasm:
	llvm-objdump -d kernel.elf

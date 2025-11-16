.PHONY: setup run disasm check-addr clean

setup:
	@./setup.sh

run:
	./run.sh

disasm:
	llvm-objdump -d kernel.elf

check-addr:
	llvm-nm kernel.elf

clean:
	rm -f kernel.elf kernel.map

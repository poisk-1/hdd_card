src = bios.asm
part = AT28C64
rom_size_kb = 8

.PHONY: all
all: bios.rom

bios.bin: $(src)
	nasm -f bin -o $@ -l bios.lst $(src)

romify: romify.c
	gcc -o $@ -DROM_SIZE_KB=$(rom_size_kb) romify.c

bios.rom: romify bios.bin
	./romify bios.bin $@

.PHONY: clean
clean:
	$(RM) bios.bin
	$(RM) bios.rom
	$(RM) romify

.PHONY: program
program: bios.rom
	minipro -p $(part) -w bios.rom

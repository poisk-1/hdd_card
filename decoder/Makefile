part = AT28C64

.PHONY: all
all: decoder.rom

make_decoder: make_decoder.c
	gcc -o $@ make_decoder.c

 decoder.rom: make_decoder
	./make_decoder $@

.PHONY: clean
clean:
	$(RM) decoder.rom
	$(RM) make_decoder

.PHONY: program
program: decoder.rom
	minipro -p $(part) -w decoder.rom

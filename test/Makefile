src = test.asm

.PHONY: all
all: test.com

test.com: $(src)
	nasm -f bin -o $@ -l test.lst $(src)

.PHONY: clean
clean:
	$(RM) test.com
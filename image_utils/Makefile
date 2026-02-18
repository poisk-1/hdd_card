.PHONY: all
all: pack unpack

pack: pack.c
	c99 -o $@ pack.c

unpack: unpack.c
	c99 -o $@ unpack.c

.PHONY: clean
clean:
	$(RM) pack unpack

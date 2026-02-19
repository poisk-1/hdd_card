.PHONY: all
all: pack unpack

pack: pack.c ../firmware/image_info.c ../firmware/image_info.h
	c99 -o $@ pack.c ../firmware/image_info.c

unpack: unpack.c ../firmware/image_info.c ../firmware/image_info.h
	c99 -o $@ unpack.c ../firmware/image_info.c

.PHONY: clean
clean:
	$(RM) pack unpack

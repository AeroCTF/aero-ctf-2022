all: arctic

.PHONY: *

arctic:
	gcc-10 -o arctic.o -c arctic.c -fno-stack-protector -O0
	gcc-10 -o arctic arctic.o ./musl-1.2.3/lib/libc.a -nostdlib -static
	strip --strip-all arctic

clean:
	rm -rf arctic arctic.o

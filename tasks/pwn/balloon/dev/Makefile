all: preload.so flag

.PHONY: *

preload.so:
	gcc -o preload.so preload.c -w -shared -fPIC -O0 -s

flag:
	gcc -o flag flag.c -s -O0

clean:
	rm preload.so flag

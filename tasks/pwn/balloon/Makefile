all: preload.so win

.PHONY: *

preload.so:
	gcc -o preload.so preload.c -w -shared -fPIC -O0 -s

win:
	gcc -o win win.c -s -O0

clean:
	rm preload.so win

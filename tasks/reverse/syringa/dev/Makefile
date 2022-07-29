all: syringa

.PHONY: *

syringa:
	g++-10 -std=c++2a -o syringa main.cpp src/* -I ./inc -s -O0
	strip --strip-all syringa

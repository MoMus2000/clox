.PHONY: run clean build

all:
	mkdir -p build
	gcc -o build/clox *.c

run:
	./build/clox

clean:
	rm -f ./build/*


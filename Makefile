.PHONY: run clean build

all:
	mkdir -p build
	gcc -o build/clox src/*.c -I ./src/include/

run:
	./build/clox $(args)

clean:
	rm -f ./build/*


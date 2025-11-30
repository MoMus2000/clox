.PHONY: run clean build

all:
	mkdir -p build
	gcc -DDEBUG_IMPLEMENTATION=1 -o build/clox src/*.c -I ./src/include/

run:
	./build/clox $(args)

clean:
	rm -f ./build/*

test:
	gcc -o build/clox_test src/*.c -I ./src/include/
	gcc -o build/test_suite tests/*c
	./build/test_suite

prod:
	mkdir -p build
	gcc -O3 -o build/clox src/*.c -I ./src/include/


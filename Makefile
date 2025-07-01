all: build

clean:
	rm -f tabletDriver

build:
	cc src/tabletDriver.c -o tabletDriver

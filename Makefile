all: build

clean:
	rm -f tabletDriver

build:
	cc src/tabletDriver.c src/argument_parser.c src/ssh.c -I/usr/include/libssh -L/usr/lib64 -lssh -g -o tabletDriver

CC = gcc
CFLAGS = -ffreestanding -fno-stack-protector -nostdlib -nostdinc -static -m32 -Wall -g -O2

hexconv: hw06.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

clean:
	rm -f hexconv
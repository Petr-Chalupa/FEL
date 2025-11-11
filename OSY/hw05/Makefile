CC = gcc
CFLAGS = -Wall -g -O2
EXTRA_CFLAGS ?= -fsanitize=address -fno-omit-frame-pointer

factory: hw05.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

clean:
	rm -f factory
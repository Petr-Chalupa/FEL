CC = gcc
CFLAGS = -Wall -g -O2
EXTRA_CFLAGS ?= -fsanitize=address -fno-omit-frame-pointer

prod-cons: hw04.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

clean:
	rm -f prod-cons
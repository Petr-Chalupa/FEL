CC = gcc
CFLAGS = -Wall

NSD_SRC = nsd_src/nsd_main.c nsd_src/nsd.c nsd_src/nd.c
NSD_HDR = nsd_src/nsd.h nsd_src/nd.h

all: nsd forkpipe

nsd: $(NSD_SRC) $(NSD_HDR)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(NSD_SRC) -o $@

forkpipe: hw03.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

clean:
	rm -f nsd forkpipe
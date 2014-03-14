CC=gcc
CFLAGS=-D_BSD_SOURCE -std=c99 -lpcap

all: pcapdump

pcapdump: pcapdump.c pcap_types.h
	$(CC) -o $@ $< $(CFLAGS)

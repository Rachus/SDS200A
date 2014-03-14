CC=gcc
CFLAGS=-D_BSD_SOURCE -std=c99 -lpcap

all: pcap2mon

pcap2mon: pcap2mon.c pcap_types.h
	$(CC) -o $@ $< $(CFLAGS)

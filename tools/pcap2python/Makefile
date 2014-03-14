CC=gcc
CFLAGS=-D_BSD_SOURCE -std=c99 -lpcap

all: pcap2python

pcap2python: pcap2python.c pcap_types.h
	$(CC) -o $@ $< $(CFLAGS)

CC=gcc
CFLAGS=-D_BSD_SOURCE -std=c99 -lpcap

all: tabelize

tabelize: tabelize.c pcap_types.h
	$(CC) -o $@ $< $(CFLAGS)

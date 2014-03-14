CC = clang
LD = clang
OXYGEN ?= doxygen
CPPFLAGS += -I/usr/include/libusb-1.0/
CFLAGS += -fpic -g
LDFLAGS += -shared -lusb-1.0

.PHONY: all clean

all: libsds200a.so

libsds200a.so: libsds200a.o
	$(LD) $< -o $@ $(CFLAGS) $(LDFLAGS)

libsds200a.o: libsds200a.c libsds200a.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

example: example.o libsds200a.so
	$(LD) -L. $< -o $@ -lsds200a

example.o: example.c libsds200a.h
	$(CC) -I. -c $< -o $@

doc: Doxyfile libsds200a.c libsds200a.h
	$(DOXYGEN) $<

clean:
	rm -f *.o *.so
	rm -r ./doc/*


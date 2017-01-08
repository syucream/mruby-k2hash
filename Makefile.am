.PHONY: all clean

SOURCES = src/mruby_k2hash.c

# support mrbgems
MRUBY_MAK_FILE = $(MRUBY_PREFIX)/build/host/lib/libmruby.flags.mak
-include $(MRUBY_MAK_FILE)

CC      = g++
AR      = ar

# configure settings related to k2hash and mruby
CFLAGS  =  -I$(K2HASH_PREFIX)/lib -I$(MRUBY_PREFIX)/include
LDFLAGS += $(MRUBY_LDFLAGS)
LIBS    =  $(K2HASH_PREFIX)/lib/.libs/libk2hash.a $(MRUBY_LIBS)


all: libmruby_k2hash.o libmruby_k2hash.a

libmruby_k2hash.o:
	$(CC) -c -fPIC $(CFLAGS) $(LDFLAGS) $(LIBS) -o libmruby_k2hash.o $(SOURCES)

libmruby_k2hash.a:
	$(AR) -r libmruby_k2hash.a libmruby_k2hash.o

clean:
	rm -f **/*.lo
	rm -f **.o
	rm -f **.a
